<?php
defined('DUNE') or define('DUNE', true); //constante de comprobacion del sitio; los includes llevan comprobacion de esta constante como primera directiva

//constantes de core
//defined('D_SUFIJO_CONTROLADOR') or define('D_SUFIJO_CONTROLADOR', '_contr'); //sufijo que llevan las clases controlador como nombre de fichero, ej.: portada_contr.php
defined('D_SUFIJO_MODELO') or define('D_SUFIJO_MODELO', '_model'); //sufijo que llevan las clases modelo como nombre de fichero, ej.: Portada_model

//constantes de la base de datos
define('D_DATOS_MOTOR', 'mysql'); //motor de la base de datos
define('D_DATOS_SERVIDOR, 'localhost'); //servidor
define('D_DATOS_DATABASE', 'sargazos_net');
define('D_DATOS_USUARIO', 'mysqluser');
define('D_DATOS_CLAVE', 'mysqluser');
define('D_DB_PREFIJOTABLAS', '');
define('D_DB_TRAZABILIDAD', true);

//constantes del servidor de correo
define('D_MAIL_SERVIDOR', 'localhost');
define('D_MAIL_USUARIO', 'Sargazos');
define('D_MAIL_CLAVE', '');
define('D_MAIL_FROM', 'info@sargazos.net');

//otras
define('D_DEBUG', true);
//defined('D_IDIOMA_DEFECTO') or define('D_IDIOMA_DEFECTO', 'es'); //lenguaje por defecto
