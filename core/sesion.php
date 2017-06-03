<?php
//inicia la sesion si no esta iniciada ya
session_start();

$bSesionCaducada = false;

//asegurando la sesion
if(empty($_SESSION['D_HTTP_USER_AGENT'])){ /* Session Fixation */
	session_regenerate_id();
	$_SESSION['D_HTTP_USER_AGENT'] = md5($_SERVER['HTTP_USER_AGENT']);
	$_SESSION['D_sesionInicio'] = time(); //momento en el que se haya iniciado la sesion
}
if($_SESSION['D_HTTP_USER_AGENT'] != md5($_SERVER['HTTP_USER_AGENT'])){ /* Session Hijacking */
	//logout
	session_destroy();
	session_unset();
	session_start();
	session_regenerate_id();
	$_SESSION['D_HTTP_USER_AGENT'] = md5($_SERVER['HTTP_USER_AGENT']);
	$_SESSION['D_sesionInicio'] = time(); //momento en el que se haya iniciado la sesion
}

//usuario logado
if(!isset($_SESSION['D_login']['id'])) $_SESSION['D_login']['id'] = false;

//$_SESSION['sesionCaducada'] = false;
if($_SESSION['D_login']['id'] && (time() - $_SESSION['D_sesionUltimaActividad'] > ini_get('session.gc_maxlifetime'))){ /* sesion caducada */
	//ini_get('session.gc_maxlifetime') specifies the number of seconds after which data will be seen as 'garbage' and potentially cleaned up. Garbage collection may occur during session start (depending on session.gc_probability and session.gc_divisor)
	$bSesionCaducada = empty($_SESSION['D_sesionUltimaActividad'])?ini_get('session.gc_maxlifetime'):$_SESSION['D_sesionUltimaActividad'];
	session_destroy();
	session_unset();
	session_start();
	session_regenerate_id();
	$_SESSION['D_HTTP_USER_AGENT'] = md5($_SERVER['HTTP_USER_AGENT']);
	$_SESSION['D_sesionInicio'] = time(); //momento en el que se haya iniciado la sesion
}

$_SESSION['D_sesionUltimaActividad'] = time(); //momento de ultima actividad

$sUrlReferer = (empty($_SESSION['D_urlReferer'])?$_SERVER['PHP_SELF']:$_SESSION['D_urlReferer']); //url previa
$_SESSION['D_urlReferer'] = $_SERVER['PHP_SELF'];
