<?php
defined('DUNE') or define('DUNE', true); //constante de comprobacion del framework; los includes llevan comprobacion de esta constante como primera directiva

//constantes de core
//defined('D_SUFIJO_CONTROLADOR') or define('D_SUFIJO_CONTROLADOR', '_contr'); //sufijo que llevan las clases controlador como nombre de fichero, ej.: portada_contr.php
defined('D_MODULO_INICIO') or define('D_MODULO_INICIO', 'portada'); //modulo a cargar en inicio, ya sea nombre de vista o de controlador
defined('D_METODO_INICIO') or define('D_METODO_INICIO', 'inicio'); //metodo a cargar por omision, si no se pasa ?modulo=metodo
defined('D_MODULO_ERROR') or define('D_MODULO_ERROR', 'error'); //modulo a cargar en error

//constantes del servidor de correo
define('D_MAIL_SERVIDOR', 'localhost');
define('D_MAIL_USUARIO', 'Sargazos');
define('D_MAIL_CLAVE', '');
define('D_MAIL_FROM', 'info@sargazos.net');

//otras
define('D_DEBUG', true);
//defined('D_IDIOMA_DEFECTO') or define('D_IDIOMA_DEFECTO', 'es'); //lenguaje por defecto
date_default_timezone_set('Europe/Madrid');

defined('CIERRE_APP') or define('CIERRE_APP', false); //si true cierra la aplicacion; debe definirse el comportamiento en mods/controladores/
