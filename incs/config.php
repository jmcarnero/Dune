<?php
define('SARGAZOS', true); //constante de comprobacion del sitio; los includes llevan comprobacion de esta constante como primera directiva

//constantes de core
define('SUFIJO_CONTROLADOR', '_contr'); //sufijo que llevan las clases controlador como nombre de fichero, ej.: portada_contr.php
define('SUFIJO_MODELO', '_model'); //sufijo que llevan las clases modelo como nombre de fichero, ej.: portada_model.php

//constantes de site
define('SITE_BASE', '/'); //directorio raiz
define('SITE_SERVER','site.local'); //servidor
define('DEBUG', false);

//constantes de la base de datos
define('DB_DATABASE', 'database');
define('DB_ENGINE', 'mysql'); //motor de la base de datos
define('DB_SERVER', 'localhost'); //servidor
define('DB_USER', 'user');
define('DB_PASSWORD', 'passwd');
define('DB_PREFIJOTABLAS', '');
define('DB_TRAZABILIDAD', false);

//constantes del servidor de correo
define('MAIL_SERVER', 'localhost');
define('MAIL_USER', 'user');
define('MAIL_PASSWORD', 'passwd');
define('MAIL_FROM', 'info@site.local');

//otras
define('DEF_LANG', 'es'); //lenguaje por defecto

