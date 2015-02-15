<?php
defined('DUNE') or define('DUNE', true); //constante de comprobacion del sitio; los includes llevan comprobacion de esta constante como primera directiva

//constantes de core
//defined('D_SUFIJO_CONTROLADOR') or define('D_SUFIJO_CONTROLADOR', '_contr'); //sufijo que llevan las clases controlador como nombre de fichero, ej.: portada_contr.php
defined('D_SUFIJO_MODELO') or define('D_SUFIJO_MODELO', '_model'); //sufijo que llevan las clases modelo como nombre de fichero, ej.: portada_model.php

//constantes de site
if($_SERVER['SERVER_NAME'] == '192.168.0.2' || $_SERVER['SERVER_NAME'] == 'itaca'){
	define('D_SITE_BASE', '~jmanuel/sargazos_net/'); //directorio raiz
	define('D_SITE_SERVER', 'itaca'); //servidor
	define('D_DB_DATABASE', 'sargazos_net');
	define('D_DB_USER', 'mysqluser');
	define('D_DEBUG', true);
}
else{
	define('D_SITE_BASE', '/'); //directorio raiz
	define('D_SITE_SERVER','sargazos.net'); //servidor
	define('D_DB_DATABASE', '28_sargazosn');
	define('D_DB_USER', '28_mysqluser');
	define('D_DEBUG', false);
}

//constantes de la base de datos
define('D_DB_ENGINE', 'mysql'); //motor de la base de datos
define('D_DB_SERVER', 'localhost'); //servidor
define('D_DB_PASSWORD', 'mysqluser');
define('D_DB_PREFIJOTABLAS', '');
define('D_DB_TRAZABILIDAD', true);

//constantes del servidor de correo
define('D_MAIL_SERVER', 'localhost');
define('D_MAIL_USER', 'Sargazos');
define('D_MAIL_PASSWORD', '');
define('D_MAIL_FROM', 'info@sargazos.net');

//otras
defined('D_DEF_LANG') or define('D_DEF_LANG', 'es'); //lenguaje por defecto
