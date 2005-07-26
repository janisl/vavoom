<html>
<head>
	<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1" />
	<link rel="stylesheet" href="styles.css" type="text/css" />
</head>
<body color="#FFFFFF" marginwidth="10">
<center>
	<img src="images/logo/vavoom.jpg" border="0" height="105" width="447" /><br />
	<span class="ad">THE MOST ADVANCED DOOM/HERETIC/HEXEN/STRIFE SOURCE PORT AROUND!</span>
</center>
<br /><br />
<?php
define('IN_PHPBB', true);
$phpbb_root_path = 'forums/';
include($phpbb_root_path . 'extension.inc');
include($phpbb_root_path . 'common.'.$phpEx);
include($phpbb_root_path . 'includes/bbcode.'.$phpEx);

//
// Go ahead and pull all data for this topic
//
$sql = "SELECT u.username, p.*,  t.topic_id, t.topic_title, t.topic_time, t.topic_replies, pt.post_text, pt.post_subject, pt.bbcode_uid
	FROM " . TOPICS_TABLE . " t, " . POSTS_TABLE . " p, " . USERS_TABLE . " u, " . POSTS_TEXT_TABLE . " pt
	WHERE p.post_id = t.topic_first_post_id
		AND pt.post_id = t.topic_first_post_id
		AND u.user_id = p.poster_id
		AND t.forum_id = 7
	ORDER BY t.topic_time DESC
	LIMIT 0, 5";
if ( !($result = mysql_query($sql)) )
{
	message_die(GENERAL_ERROR, "Could not obtain post/user information.", '', __LINE__, __FILE__, $sql);
}

$postrow = array();
if ($row = mysql_fetch_array($result))
{
	do
	{
		$postrow[] = $row;
	}
	while ($row = mysql_fetch_array($result));
	mysql_free_result($result);

	$total_posts = count($postrow);
}

//
// Okay, let's do the loop, yeah come on baby let's do the loop
// and it goes like this ...
//
for($i = 0; $i < $total_posts; $i++)
{
	$poster = $postrow[$i]['username'];

	$post_date = create_date('d.m.Y', $postrow[$i]['topic_time'], $board_config['board_timezone']);

	$topic_id = $postrow[$i]['topic_id'];

	$topic_replies = $postrow[$i]['topic_replies'];

	$post_subject = $postrow[$i]['topic_title'];

	$message = $postrow[$i]['post_text'];
	$bbcode_uid = $postrow[$i]['bbcode_uid'];

	//
	// Note! The order used for parsing the message _is_ important, moving things around could break any
	// output
	//

	//
	// If the board has HTML off but the post has HTML
	// on then we process it, else leave it alone
	//
/*	if ( !$board_config['allow_html'] )
	{
		if ( $postrow[$i]['enable_html'] )
		{
			$message = preg_replace('#(<)([\/]?.*?)(>)#is', "&lt;\\2&gt;", $message);
		}
	}*/

	//
	// Parse message and/or sig for BBCode if reqd
	//
/*	if ( $board_config['allow_bbcode'] )
	{
		if ( $bbcode_uid != '' )
		{
			$message = ( $board_config['allow_bbcode'] ) ? bbencode_second_pass($message, $bbcode_uid) : preg_replace('/\:[0-9a-z\:]+\]/si', ']', $message);
		}
	}*/

	$message = make_clickable($message);

	//
	// Parse smilies
	//
	if ( $board_config['allow_smilies'] )
	{
		if ( $postrow[$i]['enable_smilies'] )
		{
			$message = smilies_pass($message);
		}
	}

	//
	// Replace newlines (we use this rather than nl2br because
	// till recently it wasn't XHTML compliant)
	//
	$message = str_replace("\n", "\n<br />\n", $message);

	echo("<div class=\"date\">$post_subject</div><div>Posted by $poster $post_date<br/><br/>$message<br/><br/>
	<a href=\"http://www.vavoom-engine.com/forums/viewtopic.php?t=$topic_id\">$topic_replies comments</a></div><br />");
}

?>

<script language="JavaScript">
	if (navigator.appName != 'Microsoft Internet Explorer' &&
		navigator.appVersion.substr(0, 4) < 5)
	{
		document.write(
			'<span class="small">' +
			'<font color="red">WARNING: You are using ' + navigator.appName +
			' version ' + navigator.appVersion.substr(0, 4) + '<br /><br />' +
			'This site uses CSS and some other fancy new stuff, which older' +
			' Netscape versions do not understand properly. Please upgrade ' +
			'to <a target="_blank" href="http://home.netscape.com/download">Netscape 6.x+</a> ' +
			'(which shows the navigator.appVersion as 5.0) if you can, or ' +
			'use Internet Explorer 4.x+, as this site may not render or ' +
			'function properly with your current browser.</font>' +
			'</span>');
	}
</script>

</body>
</html>
