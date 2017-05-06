<?php if(!defined('DUNE')) die('...error');

$aMod['error'] = array('title' => $this->trad('Incidencia'));
$sTitle = $aMod['error']['title'];
$sMetas = '';
$sCss .= '<link rel="stylesheet" type="text/css" media="screen,projection" href="' . D_BASE_URL . 'css/error.css" />';
$sJs = '';
?><div class="error">
	<h2><?php echo $sMensaje; ?></h2>
</div>
