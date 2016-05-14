<?php

$max = test_input($_POST["max"]);
$min = test_input($_POST["min"]);
$order = test_input($_POST["model"]);
$sort = test_input($_POST["sort"]);
$screen = test_input($_POST["out"]);
$target_dir = "uploads/";
$target_file = $target_dir . basename($_FILES["uploadfile"]["name"]);
$uploadOk = 1;
if (file_exists($target_file)) {
	echo "Sorry, file already exists.";
	$uploadOk = 0;
}
if ($uploadOk == 0) {
	echo "Sorry, your file was not uploaded.";
} else {
	if (move_uploaded_file($_FILES["uploadfile"]["tmp_name"], $target_file)) {
		echo "The file ". basename($_FILES["uploadfile"]["name"]). " has been uploaded.";
	}
	else {
		echo "Sorry, there was an error uploading your file.";
	}
}
$output = fopen("out.txt","w") or die("Unable to open output file!");

#exec("./a.out -min $min -max $max $sort -order $order $target_file", $out);
exec("./a.out",$out);
$len = count($out);
if ($screen) {
	for ($x = 0; $x < $len; $x++) {
		fwrite($output, $out[$x]);
		fwrite($output,"\n");
	}
	$file="out.txt";
	if (file_exists($file)) {
	#header("Content-Disposition: attachment; filename=\"" . basename($file) . "\"");
    header("Content-Type: application/force-download");
    header('Content-Disposition: attachment; filename="'.basename($file).'"');
    header('Content-Length: ' . filesize($file));
    readfile($file);
    header("Connection: close");
    exit;
	}
}else {
	for ($x = 0; $x < $len; $x++) {
		$tmp = $out[$x];
		echo $tmp;
		echo "\n";
	}
}
	

function test_input($data) {
  $data = trim($data);
  $data = stripslashes($data);
  $data = htmlspecialchars($data);
  return $data;
}

?>