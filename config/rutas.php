<?php
/*
Enrutado de url's
Fichero de configuracion opcional

$x -> sustitucion de valores en cadena; x = [1-9]

'ruta_real' => 'expresion regular para coincidencia de url's limpias'
'?controlador=metodo&parametro1=$1&parametro2=$2' => 'controlador/metodo(/.+)(/.+)'

Las expresiones regulares se analizan sensibles a mayusculas

Es importante el orden de expresiones en el array, ya que se usara la primera coincidencia
Usar & como delimitador de fragmentos en "ruta_real", no la entidad "&amp;"
*/

$aRutas = array(
	'?portada&tag=$1' => '/portada&tag=([^/]+)/?$',
	//'?portada' => 'portada/?$',
	'?entrada&ent=$1' => '/entrada/(\d+)/?$',
	//'?$1&$2' => '/([^/]+)/([^/]+)/?$', //comodin, cualquier controlador y metodo
	//'?$1' => '/([^/]+)/?$' //comodin, cualquier controlador a su metodo por defecto
);

return $aRutas;