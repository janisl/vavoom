<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<?php

#This script is a scaled-down version of a more complex uploader script
#originally made by Sneax for the exclusive use of the RavenGames Network.
#This scaled-down version tailored for the Vavoom homepage by RambOrc.
#Copyright 2002. All rights of distribution reserved.

$files_path = "/home/vavoom/uploaded_files";

#header
function upload_header()
{
	?>
	<html>
	<head>
	<title>Vavoom file uploader</title>
	</head>
	
	<body>
	<?php
}

#footer
function upload_footer()
{
	?>
	</body>
	</html>
	<?php
}


if ($page == "")
{
	upload_header();
	?>
	<h1>Upload a file</h1>
	<br>
	<form action="upload.php?page=doupload" method="post" enctype="multipart/form-data">
	<b>File to upload:</b> <input type="file" name="userfile">
	<br><br><input type="submit" value="submit">
	</form>
	<p>If you have a file larger than 1 MB to upload, please e-mail to
	vavoom at vavoom dash engine dot com to get a temporary FTP account to 
	upload it.</p>
	<?php
	upload_footer();
}
if ($page == "doupload")
{
	upload_header();

	#Now we get some vars for our use when doing an upload
	#$userfile = the path to the uploaded file on the server (php - temp dir)
	#$userfile_name = the path the uploaded file on the client's pc
	#$userfile_size = the size of the file in bytes
	#$userfile_type = the type of the file (if the browser gives it)

	$filename = $userfile_name;
	$filename_path = $files_path."/".$filename;
	
	$i = "0";
	while (file_exists($filename_path))
	{
		$i++;
		$filename_path = $files_path."/".$i.$filename;
	}

	#Then we are going to copy it - from the temp dir on the server ($userfile)
	#to the pic dir (with the help of the var $pic_path - defined in config.inc.php
	copy($userfile, $filename_path) or die ("<b>Sorry but the file could not be uploaded. Check that you have permission to do this and that your config vars are ok.</b>");
 		
	echo "File successfully uploaded. If you want to upload another file, click <a href=\"http://www.vavoom-engine.com/upload.php\">here</a>.";
	upload_footer();
}
?>
