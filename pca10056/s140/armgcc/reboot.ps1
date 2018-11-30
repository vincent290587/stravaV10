[System.IO.Ports.SerialPort]::getportnames()
$port= new-Object System.IO.Ports.SerialPort COM15,115200,None,8,one
$port.open()
$port.WriteLine("    $DWN,17*     ")
$port.Close()