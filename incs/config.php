<?php
defined('DUNE') or define('DUNE', true); //constante de comprobacion del sitio; los includes llevan comprobacion de esta constante como primera directiva

//constantes de core
defined('D_SUFIJO_MODELO') or define('D_SUFIJO_MODELO', '_model'); //sufijo que llevan las clases modelo como nombre de fichero, ej.: Portada_model
defined('D_MODULO_INICIO') or define('D_MODULO_INICIO', 'portada'); //modulo a cargar en inicio, ya sea nombre de vista o de controlador
defined('D_METODO_INICIO') or define('D_METODO_INICIO', 'inicio'); //metodo a cargar por omision, si no se pasa ?modulo=metodo
defined('D_MODULO_ERROR') or define('D_MODULO_ERROR', 'error'); //modulo a cargar en error

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
