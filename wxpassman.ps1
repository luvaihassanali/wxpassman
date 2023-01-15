$path = "$PSScriptRoot\vc_mswu_x64"
Remove-Item "$path\export.true" -Force -ErrorAction SilentlyContinue

$data = "name,url,username,password"
$data += "`n"

$count = 0
foreach($line in Get-Content "$path\temp.txt") {
  $value = $line -split "= "
  if ($count -eq 3) {
    $data += ($value[1] + "`n")
    $count = -1;
  } else {
    $data += ($value[1] + ",") 
  }
  $count++
}

$output = ConvertFrom-Csv @$data
Remove-Item "$path\temp.csv" -Force -ErrorAction SilentlyContinue
$output | Export-Csv -Path "$path\temp.csv" -NoTypeInformation
Remove-Item "$path\temp.txt" -Force -ErrorAction SilentlyContinue