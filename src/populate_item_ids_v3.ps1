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

Write-Host "FoodReminder-Nexus itemID mapper v3"
Write-Host "Source: $sourcePath"
Write-Host ""

$items = @()
$cacheGood = $false

if (Test-Path $cachePath) {
    try {
        $cached = @(Get-Content $cachePath -Raw | ConvertFrom-Json)
        if ($cached.Count -gt 1000) {
            $items = $cached
            $cacheGood = $true
            Write-Host ("Using cached GW2 item catalog ({0} items)." -f $items.Count)
        } else {
            Write-Host "Existing catalog cache is incomplete. Rebuilding it..."
        }
    } catch {
        Write-Host "Existing catalog cache could not be read. Rebuilding it..."
    }
}

if (-not $cacheGood) {
    Write-Host "Downloading Guild Wars 2 item ID list..."
    $allIds = @(Invoke-RestMethod -Uri "https://api.guildwars2.com/v2/items")
    Write-Host ("Found {0} item IDs." -f $allIds.Count)

    $itemsList = New-Object System.Collections.ArrayList
    $batchSize = 200
    $totalBatches = [int][Math]::Ceiling($allIds.Count / [double]$batchSize)

    for ($i = 0; $i -lt $allIds.Count; $i += $batchSize) {
        $end = [Math]::Min($i + $batchSize - 1, $allIds.Count - 1)
        $batch = @($allIds[$i..$end])
        $ids = $batch -join ","
        $uri = "https://api.guildwars2.com/v2/items?ids=$ids&lang=en"

        $batchItems = @(Invoke-RestMethod -Uri $uri)
        foreach ($item in $batchItems) {
            [void]$itemsList.Add($item)
        }

        $batchNumber = [int]([Math]::Floor($i / $batchSize) + 1)
        Write-Host ("Downloaded batch {0}/{1} ({2} items total)" -f $batchNumber, $totalBatches, $itemsList.Count)
    }

    # Convert ArrayList safely to a normal PowerShell array.
    $items = @($itemsList | ForEach-Object { $_ })

    Write-Host ("Saving catalog cache ({0} items)..." -f $items.Count)
    $items | ConvertTo-Json -Depth 20 | Set-Content $cachePath -Encoding UTF8
    Write-Host ("Catalog cached: {0}" -f $cachePath)
}

$nameIndex = @{}
foreach ($item in $items) {
    if ($item.type -ne "Consumable" -or [string]::IsNullOrWhiteSpace($item.name)) {
        continue
    }

    if (-not $nameIndex.ContainsKey($item.name)) {
        $nameIndex[$item.name] = @()
    }
    $nameIndex[$item.name] += $item
}

$text = Get-Content $sourcePath -Raw

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
            effect_id = $effectID; label = $label; name = $name
            status = "preserved_existing"; item_id = $existingItemID
            matched_api_name = $name
        })
        return $m.Value
    }

    if ($name.StartsWith("Unknown (") -or $label -eq "Unknown") {
        $script:unknown++
        $script:results.Add([pscustomobject]@{
            effect_id = $effectID; label = $label; name = $name
            status = "skipped_unknown"; item_id = 0; matched_api_name = ""
        })
        return $m.Value
    }

    $matches = @()
    if ($nameIndex.ContainsKey($name)) {
        $matches = @($nameIndex[$name])
    }

    if ($matches.Count -eq 0) {
        $script:notFound++
        $script:results.Add([pscustomobject]@{
            effect_id = $effectID; label = $label; name = $name
            status = "not_found"; item_id = 0; matched_api_name = ""
        })
        return $m.Value
    }

    if ($matches.Count -gt 1) {
        $script:ambiguous++
        $display = ($matches | ForEach-Object { "$($_.name) [$($_.id)]" }) -join " | "
        $script:results.Add([pscustomobject]@{
            effect_id = $effectID; label = $label; name = $name
            status = "ambiguous"; item_id = 0; matched_api_name = $display
        })
        return $m.Value
    }

    $item = $matches[0]
    $itemID = [int]$item.id
    $script:patched++

    $script:results.Add([pscustomobject]@{
        effect_id = $effectID; label = $label; name = $name
        status = "patched"; item_id = $itemID; matched_api_name = $item.name
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
