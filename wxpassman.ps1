Import-Module importexcel

cd "C:\Users\luv\Desktop"

$data = "name,url,username,password"
$data += "`n"

$count = 0
foreach($line in Get-Content .\temp.txt) {
  $value = $line -split "= "
  if ($count -eq 3) {
    $data += ($value[1] + "`n")
    $count = -1;
  } else {
    $data += ($value[1] + ",") 
  }
  $count++
}
$data
$output = ConvertFrom-Csv @$data
$output
Remove-Item .\temp.xlsx -Force
$output | Export-Excel .\temp.xlsx