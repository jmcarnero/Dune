<?php
//inicia la sesion si no esta iniciada ya
session_start();

$bSesionCaducada = false;

//asegurando la sesion
if(empty($_SESSION['HTTP_USER_AGENT'])){ /* Session Fixation */
	session_regenerate_id();
	$_SESSION['HTTP_USER_AGENT'] = md5($_SERVER['HTTP_USER_AGENT']);
	$_SESSION['sesionInicio'] = time(); //momento en el que se haya iniciado la sesion
}
if($_SESSION['HTTP_USER_AGENT'] != md5($_SERVER['HTTP_USER_AGENT'])){ /* Session Hijacking */
	//logout
	session_destroy();
	session_unset();
	session_start();
	session_regenerate_id();
	$_SESSION['HTTP_USER_AGENT'] = md5($_SERVER['HTTP_USER_AGENT']);
	$_SESSION['sesionInicio'] = time(); //momento en el que se haya iniciado la sesion
}

//usuario logado
if(!isset($_SESSION['estaLogado'])) $_SESSION['estaLogado'] = false;
/*
'estaLogadoAviso' => string 'ko' (length=2)
'caduca' => int 0
'estaLogado' => int 1
'idUsuario' => string '1' (length=1)
'login' => string 'jm_carnero@sargazos.net' (length=23)
'clave' => string 'admin' (length=5)
'nombre' => string 'José M.' (length=8)
'apellidos' => string 'Carnero' (length=7)
'email' => string 'jm_carnero@sargazos.net' (length=23)
'activo' => string '1' (length=1)
*/

//$_SESSION['sesionCaducada'] = false;
if($_SESSION['estaLogado'] && (time() - $_SESSION['sesionUltimaActividad'] > ini_get('session.gc_maxlifetime'))){ /* sesion caducada */
	//ini_get('session.gc_maxlifetime') specifies the number of seconds after which data will be seen as 'garbage' and potentially cleaned up. Garbage collection may occur during session start (depending on session.gc_probability and session.gc_divisor)
	$bSesionCaducada = empty($_SESSION['sesionUltimaActividad'])?ini_get('session.gc_maxlifetime'):$_SESSION['sesionUltimaActividad'];
	session_destroy();
	session_unset();
	session_start();
	session_regenerate_id();
	$_SESSION['HTTP_USER_AGENT'] = md5($_SERVER['HTTP_USER_AGENT']);
	$_SESSION['sesionInicio'] = time(); //momento en el que se haya iniciado la sesion
}

$_SESSION['sesionUltimaActividad'] = time(); //momento de ultima actividad

$sUrlReferer = (empty($_SESSION['urlReferer'])?$_SERVER['PHP_SELF']:$_SESSION['urlReferer']); //url previa
$_SESSION['urlReferer'] = $_SERVER['PHP_SELF'];
