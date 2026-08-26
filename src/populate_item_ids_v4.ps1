param(
    [string]$Source = "ConsumableData.cpp",
    [switch]$DryRun
)

$ErrorActionPreference = "Stop"

if (-not (Test-Path $Source)) {
    throw "Source file not found: $Source"
}

$sourcePath = (Resolve-Path $Source).Path
$sourceDir = Split-Path $sourcePath -Parent
$reportPath = Join-Path $sourceDir "item_id_mapping_report.csv"
$cachePath = Join-Path $sourceDir "gw2_items_catalog.json"
$resumePath = Join-Path $sourceDir "gw2_items_catalog_resume.ndjson"

Write-Host "FoodReminder-Nexus itemID mapper v4"
Write-Host "Source: $sourcePath"
Write-Host ""

function Get-AllItemIds {
    $raw = (Invoke-WebRequest -UseBasicParsing -Uri "https://api.guildwars2.com/v2/items").Content
    $ids = $raw | ConvertFrom-Json
    return $ids
}

function Get-ItemBatch([int[]]$BatchIds) {
    $idsText = $BatchIds -join ","
    $uri = "https://api.guildwars2.com/v2/items?ids=$idsText&lang=en"
    $raw = (Invoke-WebRequest -UseBasicParsing -Uri $uri).Content
    $batchItems = $raw | ConvertFrom-Json
    return $batchItems
}

$items = $null
$cacheGood = $false

if (Test-Path $cachePath) {
    try {
        $cached = Get-Content $cachePath -Raw | ConvertFrom-Json
        if ($cached.Count -gt 1000) {
            $items = $cached
            $cacheGood = $true
            Write-Host ("Using cached GW2 item catalog ({0} items)." -f $items.Count)
        }
        else {
            Write-Host "Existing catalog cache is incomplete; ignoring it."
        }
    }
    catch {
        Write-Host "Existing catalog cache could not be read; ignoring it."
    }
}

if (-not $cacheGood) {
    Write-Host "Reading Guild Wars 2 item ID list..."
    $allIds = Get-AllItemIds
    Write-Host ("Found {0} item IDs." -f $allIds.Count)

    # Resume support: every successful item is written to NDJSON as it arrives.
    $completedItems = New-Object System.Collections.ArrayList
    $completedIds = @{}

    if (Test-Path $resumePath) {
        Write-Host "Found resume cache. Loading previously downloaded items..."
        foreach ($line in Get-Content $resumePath) {
            if ([string]::IsNullOrWhiteSpace($line)) { continue }
            try {
                $obj = $line | ConvertFrom-Json
                [void]$completedItems.Add($obj)
                $completedIds[[int]$obj.id] = $true
            }
            catch {
                # Ignore malformed/incomplete last line.
            }
        }
        Write-Host ("Resume cache contains {0} items." -f $completedItems.Count)
    }

    $missingIds = New-Object System.Collections.ArrayList
    foreach ($id in $allIds) {
        $iid = [int]$id
        if (-not $completedIds.ContainsKey($iid)) {
            [void]$missingIds.Add($iid)
        }
    }

    $batchSize = 200
    $totalBatches = [int][Math]::Ceiling($missingIds.Count / [double]$batchSize)

    if ($missingIds.Count -gt 0) {
        Write-Host ("Need to download {0} items in {1} batches." -f $missingIds.Count, $totalBatches)

        for ($i = 0; $i -lt $missingIds.Count; $i += $batchSize) {
            $end = [Math]::Min($i + $batchSize - 1, $missingIds.Count - 1)

            # Build a plain int[] explicitly.
            [int[]]$batchIds = @()
            for ($j = $i; $j -le $end; $j++) {
                $batchIds += [int]$missingIds[$j]
            }

            $batchItems = Get-ItemBatch $batchIds

            foreach ($item in $batchItems) {
                [void]$completedItems.Add($item)

                # Save each item immediately so this run can resume if interrupted.
                ($item | ConvertTo-Json -Depth 20 -Compress) | Add-Content $resumePath -Encoding UTF8
            }

            $batchNumber = [int]([Math]::Floor($i / $batchSize) + 1)
            Write-Host ("Downloaded batch {0}/{1} ({2} items total cached)" -f $batchNumber, $totalBatches, $completedItems.Count)
        }
    }

    Write-Host ("Final catalog contains {0} items." -f $completedItems.Count)

    # Convert to a normal object array without the problematic @($genericList) conversion.
    $items = $completedItems.ToArray()

    Write-Host "Saving final catalog cache..."
    $items | ConvertTo-Json -Depth 20 | Set-Content $cachePath -Encoding UTF8
    Write-Host ("Catalog cached: {0}" -f $cachePath)

    # Final cache is good; resume file is no longer needed.
    if (Test-Path $resumePath) {
        Remove-Item $resumePath
    }
}

# Build exact English-name index for Consumable items.
$nameIndex = @{}
foreach ($item in $items) {
    if ($item.type -ne "Consumable" -or [string]::IsNullOrWhiteSpace($item.name)) {
        continue
    }

    if (-not $nameIndex.ContainsKey($item.name)) {
        $nameIndex[$item.name] = New-Object System.Collections.ArrayList
    }
    [void]$nameIndex[$item.name].Add($item)
}

$text = Get-Content $sourcePath -Raw

# Matches compact rows like:
# { 17825, { "Power", "Bowl ...", "+100 Power\n+70 Ferocity" } }
# and already-populated rows with a fourth itemID field.
$cppString = '"(?:\\.|[^"\\])*"'
$pattern = "\{\s*(?<effect>\d+)\s*,\s*\{\s*(?<label>$cppString)\s*,\s*(?<name>$cppString)\s*,\s*(?<effects>$cppString)\s*(?:,\s*(?<itemid>\d+)\s*)?\}\s*\}"

$results = New-Object System.Collections.Generic.List[object]
$patched = 0
$preserved = 0
$unknown = 0
$notFound = 0
$ambiguous = 0

$evaluator = {
    param($m)

    $effectID = [int]$m.Groups["effect"].Value
    $labelRaw = $m.Groups["label"].Value
    $nameRaw = $m.Groups["name"].Value
    $effectsRaw = $m.Groups["effects"].Value
    $existingRaw = $m.Groups["itemid"].Value

    $label = $labelRaw.Substring(1, $labelRaw.Length - 2)
    $name = $nameRaw.Substring(1, $nameRaw.Length - 2)

    $existingItemID = 0
    if (-not [string]::IsNullOrWhiteSpace($existingRaw)) {
        $existingItemID = [int]$existingRaw
    }

    if ($existingItemID -ne 0) {
        $script:preserved++
        $script:results.Add([pscustomobject]@{
            effect_id = $effectID
            label = $label
            name = $name
            status = "preserved_existing"
            item_id = $existingItemID
            matched_api_name = $name
        })
        return $m.Value
    }

    if ($name.StartsWith("Unknown (") -or $label -eq "Unknown") {
        $script:unknown++
        $script:results.Add([pscustomobject]@{
            effect_id = $effectID
            label = $label
            name = $name
            status = "skipped_unknown"
            item_id = 0
            matched_api_name = ""
        })
        return $m.Value
    }

    $matches = @()
    if ($nameIndex.ContainsKey($name)) {
        $matches = $nameIndex[$name].ToArray()
    }

    if ($matches.Count -eq 0) {
        $script:notFound++
        $script:results.Add([pscustomobject]@{
            effect_id = $effectID
            label = $label
            name = $name
            status = "not_found"
            item_id = 0
            matched_api_name = ""
        })
        return $m.Value
    }

    if ($matches.Count -gt 1) {
        $script:ambiguous++
        $display = ($matches | ForEach-Object { "$($_.name) [$($_.id)]" }) -join " | "
        $script:results.Add([pscustomobject]@{
            effect_id = $effectID
            label = $label
            name = $name
            status = "ambiguous"
            item_id = 0
            matched_api_name = $display
        })
        return $m.Value
    }

    $item = $matches[0]
    $itemID = [int]$item.id
    $script:patched++

    $script:results.Add([pscustomobject]@{
        effect_id = $effectID
        label = $label
        name = $name
        status = "patched"
        item_id = $itemID
        matched_api_name = $item.name
    })

    return "{ $effectID, { $labelRaw, $nameRaw, $effectsRaw, $itemID } }"
}

$updated = [regex]::Replace($text, $pattern, $evaluator)

$results | Export-Csv $reportPath -NoTypeInformation -Encoding UTF8

Write-Host ""
Write-Host "Item ID mapping summary"
Write-Host "-----------------------"
Write-Host "Database entries examined: $($results.Count)"
Write-Host "New unique matches:       $patched"
Write-Host "Existing IDs preserved:   $preserved"
Write-Host "Unknown entries skipped:  $unknown"
Write-Host "Not found:                $notFound"
Write-Host "Ambiguous:                $ambiguous"
Write-Host "Report:                   $reportPath"

if ($DryRun) {
    Write-Host ""
    Write-Host "DRY RUN: ConsumableData.cpp was NOT changed."
    exit 0
}

if ($updated -eq $text) {
    Write-Host ""
    Write-Host "No source changes were required."
    exit 0
}

$stamp = Get-Date -Format "yyyyMMdd_HHmmss"
$backupPath = "$sourcePath.$stamp.bak"
Copy-Item $sourcePath $backupPath
Set-Content $sourcePath $updated -Encoding UTF8

Write-Host ""
Write-Host "Backup:         $backupPath"
Write-Host "Updated source: $sourcePath"
Write-Host ""
Write-Host "NEXT: inspect item_id_mapping_report.csv, then Rebuild Solution."
