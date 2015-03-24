<?php
# Copyright (C) 2009 José M. Carnero <jm_carnero@sargazos.net>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
# http://www.gnu.org/copyleft/gpl.html

/**
\mainpage
 * SQLazo
\section Description
 * \~spanish Conexión y consultas a bases de datos.
 * \~english Database connetion and querys.\~
 *
 * @author José M. Carnero
 * @version 2.5
 * @license http://www.gnu.org/copyleft/gpl.html
 * @package Dune
 */

/*
 * Constante con el directorio donde se encuentran las clases derivadas y/o dependientes de esta.
 */
if(!defined('CLASSES_DIR')) define('CLASSES_DIR',dirname(__FILE__));

/**
 * \~spanish Selector de tipo de base de datos con la que conectar.
 * \~english Database type selector\~
 *
 * @access public
 * @param string $motor \~spanish Motor de base de datos\~english Database engine\~
 * @return object Objeto sqlazo
 */
function &sqlazo_sel($motor = 'mysql', $servidor = 'localhost', $usuario = 'root', $password = '', $ddbb = false){
	if($motor == 'mysql' && class_exists('mysqli')) $motor = 'mysqli';

	$class = 'sqlazo_'.$motor;
	include_once(CLASSES_DIR.'/class.sqlazo_trazabilidad.inc');
	include_once(CLASSES_DIR.'/class.'.$class.'.inc'); //si no existe el fichero a incluir no se devuelve error, se controla a continuacion

	if(!class_exists('sqlazo_trazabilidad') || !class_exists($class)){
		die('No se puede usar el tipo de base de datos -> '.$class."\n");
		return(false);
	}

	$obj = new $class($servidor, $usuario, $password, $ddbb);
	return($obj);
}

/**
 * \~spanish Clase para conexion y consultas a bases de datos.
 * De momento solo MYSQL.
 *
 * Motores soportados ("$this->sMotor"):
 * mysql -> MySQL 4, 5
 * postgresql -> PostgreSQL
 * #core PHP 4
 * \~english Database connection and querys\~
 *
 * \code{.php}
include_once("./class.sqlazo.inc");

$oTest = sqlazo_sel(DB_ENGINE, DB_SERVER, DB_USER, DB_PASSWORD, DB_DATABASE);
//$oTest->conectar(DB_SERVER, DB_USER, DB_PASSWORD, DB_DATABASE); //se puede omitir si se han pasado los parametros de conexion al constructor
//$oTest->sPrefijo = DB_TABLESPREFIX;

$oTest->consulta("SELECT * FROM usuarios");

while($oTest->leeFila()){
	echo($oTest->aFila['login']."\n");
}

//var_dump($oTest->aTiempos);

$oTest->desconectar();
 * \endcode
 *
 * @since 2008-03-01
 * @abstract
 */
class sqlazo{

	/**
	 * \~spanish Campos de la consulta, en la forma: ('nombre' => nombre, 'tipo' => tipo, 'long' => longitud, 'flags' => flags) (para cada campo de los resultados).
	 * \~english Fields in query.\~
	 *
	 * @var array
	 */
	var $aCampos;

	//var $sCharset; //codificacion en que se lanzaran las querys; a vacio usara la que tenga la base de datos; valores: utf-8,
	//var $sCollation; //collation en que se lanzaran las querys; vacio usara la que tenga la base de datos; util para evitar problemas en las comparaciones de cadenas de distintas tablas

	/**
	 * \~spanish Indica si se ha conseguido conectar o no a la base de datos.
	 * \~english True if connected.\~
	 *
	 * @var boolean
	 * @access private
	 */
	var $bConectado;

	/**
	 * \~spanish Base de datos a usar (schema).
	 * \~english Database schema to use.\~
	 *
	 * @var string
	 */
	var $sDDBB;

	/**
	 * \~spanish Clave del usuario.
	 * \~english Database password.\~
	 *
	 * @var string
	 */
	var $sDDBBPassword;

	/**
	 * \~spanish Puerto del servidor de base de datos.
	 * \~english Database server port.\~
	 *
	 * @var integer
	 * @see sqlazo::setDDBBPuerto($puerto)
	 */
	var $sDDBBPuerto;

	/**
	 * \~spanish Servidor de base de datos; nombre cualificado o IP.
	 * \~english Database server, FQDN or IP.\~
	 *
	 * @var string
	 */
	var $sDDBBServidor;

	/**
	 * \~spanish Usuario de la base de datos.
	 * \~english Database user.\~
	 *
	 * @var string
	 */
	var $sDDBBUsuario;

	/**
	 * \~spanish Control de errores.
	 * \~english Error control.\~
	 *
	 * @access private
	 * @see sqlazo::getError(),sqlazo::setError()
	 * @var array
	 */
	var $aError;

	/**
	 * \~spanish Mensajes de error la clase, a fin de que puedan ser traducidos/modificados; las secuencias %s deben dejarse ya que corresponden a variables que seran luego sustituidas, si es necesario poner un % se pondra como %%
	 * \~english Error messages.\~
	 *
	 * @var array
	 * @see sqlazo::setErrorMensaje($clave, $valor),sqlazo::getErrorMensaje($clave)
	 */
	var $aErrorMensajes;

	/**
	 * \~spanish Una fila de resultados.
	 * \~english One result row.\~
	 *
	 * @var array
	 */
	var $aFila;

	/**
	 * \~spanish Todas las filas de resultados.
	 * \~english All result rows.\~
	 *
	 * @var array
	 */
	var $aFilaTodos;

	/**
	 * \~spanish Motor de base de datos (mysql, postgresql).
	 * \~english Database engine\~
	 *
	 * @var boolean
	 * @access private
	 * @see sqlazo::getMotor()
	 */
	var $sMotor;

	/**
	 * \~spanish Prefijo para las tablas.
	 * \~english Tables prefix.\~
	 *
	 * @var string
	 */
	var $sPrefijo;

	/**
	 * \~spanish Identificador de prefijo para las tablas que sera sustituido con $this->sPrefijo, por defecto '_p_'; no usar ni % ni $
	 * \~english Prefix identifier, "_p_" is the default value; don't use % or $\~
	 *
	 * @var string
	 */
	var $sPrefijoId;

	/**
	 * \~spanish SQL query completa, pasar entre comillas dobles ("); se envia a las funciones como cadena, cada "%x" implica una sustitucion con un valor del array pasado ver "consulta" y "query").
	 * \~english SQL Query, %x is a placeholder, if you need a literal % write %%\~
	 *
	 * @var string
	 */
	var $sQuery;

	//var $sQueryTipoArray; //define que tipo de array devolvera una query: dos (MYSQL_BOTH) -> ambos tipos de array, aso (MYSQL_ASSOC) -> asociativo, num (MYSQL_NUM) -> numerico; por defecto (si no se define explicitamente valor): ambos; es valido asignandole las constantes entre parentesis (naturales de PHP)

	/**
	 * \~spanish Subtipo de query que se esta realizando, para consultas alter, create o drop: table, database
	 * \~english Query subtype.\~
	 *
	 * @var string
	 */
	var $sSubTipoQuery;

	/**
	 * \~spanish Tablas implicadas en la consulta.
	 * \~english Tables in query.\~
	 *
	 * @var array
	 */
	var $aTablas;

	/**
	 * \~spanish Cadena de texto con tags HTML permitidos (en consultas insert/update, en la forma '<p><strong>[...]'; 'todos' -> permite todos los tags (comportamiento por defecto), '' -> (o sin valor) elimina todos los tags.
	 * \~english For insert/update, HTML tags allowed.\~
	 *
	 * @var string
	 */
	var $sTagsPermitidosIns;

	/**
	 * \~spanish Cadena de texto con tags html permitidos (en consultas que devuelvan datos, como select), en la forma '<p><strong>[...]'; 'todos' -> permite todos los tags, '' -> (o sin valor) elimina todos los tags (comportamiento por defecto).
	 * \~english For select, HTML tags allowed.\~
	 *
	 * @var string
	 */
	var $sTagsPermitidosSel;

	/**
	 * \~spanish Tiempos empleado en consultas; asociativo:
	 * Conectar -> tiempo en conectar a servidor,
	 * SeleccionarDDBB -> tiempo en seleccionar base de datos,
	 * $this->sTipoQuery -> tiempo en realizar consulta,
	 * [fila][] -> tiempo para el recuento de resultados, 1 por fila de resultados, todos con respecto a un mismo tiempo inicial
	 * totalLimit -> tiempo para el recuento de resultados en consultas tipo SELECT limitadas
	 * filasTodos -> tiempo para rellenado del array "$aFilaTodos"
	 * total -> suma de tiempos de todas las operaciones
	 * \~english Times used.\~
	 *
	 * @var array
	 */
	var $aTiempos;

	/**
	 * \~spanish Tipo de query que se esta realizando: select, insert, update, delete, ...
	 * \~english Query type: select, insert, update, delete, ...\~
	 *
	 * @var string
	 */
	var $sTipoQuery;

	/**
	 * \~spanish Total de registros encontrados (select o similar) o afectados (insert, update, etc.)
	 * \~english Total rows.\~
	 *
	 * @var integer
	 */
	var $iTotalFilas;

	/**
	 * \~spanish Ultima id insertada en campos "autoincrement"; false mientras no haya inserciones.
	 * \~english Last insert id.\~
	 *
	 * @var string
	 * @todo puede devolver valor incorrecto si el campo id de la tabla es "BIGINT"
	 */
	var $sUltimaId;

	/**
	 * \~spanish Extrae la codificacion de la pagina en la que se esta ejecutando el objeto
	 * y la convierte a un formato reconocido por la base de datos.
	 * \~english Codification of the connection.\~
	 *
	 * @access protected
	 * @param string $tipo \~spanish Tipo de dato devuelto\~english Data type returned\~
	 * @return string
	 */
	protected function codificacion($tipo = 'charset'){
		//extraer codificacion de la pagina actual de las cabeceras enviadas al navegador
		if(!isset($this->sCharset)){
			$this->sCharset = '';
			if(!isset($this->sCollation)) $this->sCollation = '';
		}
		$aCodificacion = array();

		//necesita php>=4.3.0
		if(function_exists('headers_list')) $aCodificacion = headers_list();
		elseif(function_exists('apache_response_headers')) $aCodificacion = apache_response_headers();

		//lista de charset: SHOW CHARACTER SET (SQL en MYSQL)
		$codificacion = '';
		foreach($aCodificacion as $codificacion){
			//if(strpos($codificacion, 'charset=') !== false) $this->sCharset = str_replace('"', '', substr($codificacion, strpos($codificacion, 'charset=')+8));
			if(strpos($codificacion, 'charset=') !== false) break;
		}

		if(strpos(strtolower($codificacion), 'utf') !== false){
			if($tipo == 'charset') return('utf8');
			elseif($tipo == 'collation') return('utf8_general_ci');
		}
		elseif(strpos(strtolower($codificacion), 'iso-8859-1') !== false || strpos(strtolower($codificacion), 'iso-8859-15') !== false){
			if($tipo == 'charset') return('latin1');
			elseif($tipo == 'collation') return('latin1_spanish_ci');
		}
		//TODO revisar funcionamiento; valores que devuelve headers_list() y demas

		return(false);
	}

	/**
	 * \~spanish Conexion al servidor.
	 * Selecciona automaticamente la base de datos,
	 * llamar a "seleccionarDDBB()" cambiando la propiedad "$this->sDDBB"
	 * para usar otra base de datos con esta misma conexion.
	 * \~english Server connect.\~
	 *
	 * @access public
	 * @param string $servidor \~spanish Servidor\~english Server\~
	 * @param string $usuario \~spanish Usuario\~english User\~
	 * @param string $clave \~spanish Clave\~english Password\~
	 * @param string $db \~spanish Schema (base de datos)\~english Schema\~
	 * @return boolean
	 * @abstract
	 */
	public function conectar($servidor = false, $usuario = false, $clave = false, $db = false){}

	/**
	 * \~spanish Consulta a la base de datos, (si "SELECT", necesita de "ddbb::leeFila()" para empezar a devolver resultados).
	 * \~english Runs the query.\~
	 *
	 * @access public
	 * @param string $query Query SQL
	 * @param array $pars \~spanish Parametros de la query\~english Query parameters\~
	 * @param integer $cantidad \~spanish Numero de registros a devolver\~english Number of rows to return\~
	 * @param integer $inicial \~spanish Registro inicial a devolver\~english Initial row to return\~
	 * @return boolean
	 * @abstract
	 */
	public function consulta($query = false, $pars = array(), $cantidad = 0, $inicial = 0){}

	/**
	 * \~spanish Desconexion del servidor.
	 * \~english Disconnect server.\~
	 *
	 * @access public
	 * @return boolean
	 * @abstract
	 */
	public function desconectar(){}

	/**
	 * \~spanish Devuelve los errores que se hayan producido.
	 * \~english Returns errors.\~
	 *
	 * @access public
	 * @return array
	 */
	public function getError(){
		return($this->aError);
	}

	/**
	 * \~spanish Muestra el texto asignado a un error.
	 * Si no se pasa ninguna clave se muestran todos.
	 * \~english Shows error texts.\~
	 *
	 * @todo mostrar la clave de un texto de mensaje de error (invirtiendo el array y un foreach para buscar coincidencia de texto)
	 * @access public
	 * @param string $clave \~spanish Clave del mensaje de error\~english Error message key\~
	 * @return string
	 */
	//TODO mostrar la clave de un texto de mensaje de error (invirtiendo el array y un foreach para buscar coincidencia de texto)
	public function getErrorMensaje($clave = false){
		if($clave === false) return($this->aErrorMensajes);
		return($this->aErrorMensajes[$clave]);
	}

	/**
	 * \~spanish Devuelve el actual tipo de base de datos usado.
	 * \~english Returns database engine.\~
	 *
	 * @access public
	 * @return string
	 */
	public function getMotor(){
		return($this->sMotor);
	}

	/**
	 * \~spanish Contador de instancias de la clase;
	 * se usa al cerrar la conexion a la base de datos,
	 * para no dejar instancias sin acceso.
	 * \~english Class instances counter.\~
	 *
	 * @access protected
	 * @param integer $inc Incremento
	 * @return integer
	 */
	protected function instCount($inc = 1){
		if(!isset($iInstCount)){
			static $iInstCount = 0;
		}
		if($inc == 1) $iInstCount++;
		if($inc == -1) $iInstCount--;
		//echo('$iInstCount --> '.$iInstCount);
		return($iInstCount);
	}

	/**
	 * \~spanish Lee una fila de resultados.
	 * \~english Returns one result row.\~
	 *
	 * @access public
	 * @return boolean
	 * @abstract
	 */
	public function leeFila(){}

	/**
	 * \~spanish Crea un array con todas las filas de resultados.
	 * \~english Returns all results rows.\~
	 *
	 * @access public
	 * @return boolean
	 * @abstract
	 */
	public function leeFilasTodos(){}

	/**
	 * \~spanish Devuelve microtime en segundos
	 * \~english Converts microtime to seconds\~
	 *
	 * @access protected
	 * @param boolean $float \~spanish Si true devuelve un numero flotante, si false devuelve formateado "legible para humanos" (parecido a ISO 8601)\~english True returns float, false return readable time\~
	 * @return float
	 */
	protected function microtimeSeg($float = true){
		list($mseg, $seg) = explode(" ", microtime());

		if($float) return((float)$mseg + (float)$seg);
		else return(date('Y-m-d H:i:s', $seg).substr($mseg, 1));
	}

	/**
	 * \~spanish Lanza una modificacion de la consulta antes de lanzar la consulta en si para recuperar algun tipo de valor de forma automatizada (maximo, minimo, etc) //pensada, por ahora, para SQL y consultas "select".
	 * \~english Modify the query before run'em.\~
	 *
	 * @access public
	 * @param string $tipo \~spanish Tipo de modificacion ('max' => maximo valor de $campo)\~english Modification type\~
	 * @param string $campo \~spanish Campo sobre el que se operara la modificacion\~english Field to modify\~
	 * @return mixed
	 */
	public function operaQuery($tipo = 'max', $campo = false){
		$ret = false;

		include_once(dirname(__FILE__).'/class.parse_sql.inc');

		if(!class_exists('parse_sql')){
			$this->setError(__METHOD__.'1');
			return(false);
		}

		$aFragmentos = parse_sql::ParseString($this->sQuery)->getArray();

		switch($tipo){
			/*calcular el maximo de un campo
			SELECT max(CAST(REPLACE(es.population, '.', '') AS unsigned)) AS peso FROM datos_poblacion_usa es;*/
			case 'max':
				if(stripos($aFragmentos['select'], 'select') !== 0){ //solo para consultas select
					$this->setError(__METHOD__.'2');
					return(false);
				}
				$aFragmentos['sql'] = 'SELECT MAX('.$campo.') AS valor ';
				if($this->consulta($aFragmentos['sql'].$aFragmentos['from']) && $this->leeFila()){
					return($this->aFila['valor']);
				}
				else{
					$this->setError(__METHOD__.'3', $this->sQuery);
					return(false);
				}
				break;
		}

		return(false); //error desconocido
	}

	/**
	 * \~spanish Construir query.
	 * si ya se ha asignado una query mediante la propiedad "sQuery" y no se desea cambiar NO llamar a esta funcion y llamar al metodo "consulta" sin parametros
	 * ej: $obj->fQuery("SELECT * FROM tabla WHERE campo = %s AND b = %s", array('1', '2'))
	 * cuando se requiera el comodin SQL "%" escribirlo doble "%%"; ej: $obj->consulta("SELECT * FROM tabla WHERE campo='%%%s%%'", array('hola')) ->producira la salida: "SELECT * FROM tabla WHERE campo='%hola%'"
	 *
	 * %s -> tomara del array de parametros una cadena de caracteres (en la misma posicion relativa)
	 * %d -> entero con signo
	 * %u -> entero sin signo
	 * %F -> flotante (PHP>4.3)
	 * \~english Build query.
	 * ex: $obj->fQuery("SELECT * FROM oneTable WHERE oneField = %s AND otherField = %s", array('1', '2'))
	 * %s -> string
	 * %d -> integer, signed
	 * %u -> integer, unsigned
	 * %F -> float
	 * \~
	 *
	 * @access protected
	 * @param string $query Query SQL
	 * @param array $pars \~spanish Parametros de la query\~english Query parameters\~
	 * @param integer $cantidad \~spanish Numero de registros a devolver\~english Number of rows to return\~
	 * @param integer $inicial \~spanish Registro inicial a devolver\~english Initial row to return\~
	 * @return void
	 * @abstract
	 */
	protected function query($query, $pars = array(), $cantidad = 0, $inicial = 0){}

	/**
	 * \~spanish Recolocar puntero en el array de resultados (solo para SELECT).
	 * \~english Pointer position.\~
	 *
	 * @access public
	 * @param integer $pos \~spanish Posicion puntero\~english Pointer position\~
	 * @return boolean
	 * @abstract
	 */
	public function recPuntero($pos = 0){}

	/**
	 * \~spanish Seleccion de base de datos.
	 * \~english Database selection.\~
	 *
	 * @todo comprobar que el servidor admite mayusculas para el nombre de la base de datos, si no cambiarla a minusculas
	 * @access public
	 * @param string $db \~spanish Base de datos\~english Database\~
	 * @return boolean
	 * @abstract
	 */
	public function seleccionarDDBB($db = false){}

	/**
	 * \~spanish Establece el puerto con el que se conectara.
	 * \~english Sets database port.\~
	 *
	 * @access public
	 * @param integer $puerto \~spanish Número de puerto\~english Port number\~
	 * @return boolean
	 */
	public function setDDBBPuerto($puerto){
		$sTemp = $puerto;
		if(strpos($this->sDDBBServidor, ':') !== false){ //limpia la direccion del servidor si ya lleva puerto, y lo recupera para comprobar si es el mismo que se ha pasado
			$sTemp = substr($this->sDDBBServidor, strpos($this->sDDBBServidor, ':')+1);
			$this->sDDBBServidor = substr($this->sDDBBServidor, 0, strpos($this->sDDBBServidor, ':'));
		}
		if($sTemp != $puerto){
			if(empty($puerto)) $puerto = $sTemp;
			else{
				$this->setError(__METHOD__, $sTemp, $puerto, $puerto);
				//return(false);
			}
		}

		if($puerto === false) return(false);
		$this->sDDBBPuerto = $puerto;

		return(true);
	}

	/**
	 * \~spanish Guarda los errores que se hayan producido.
	 * \~english Saves errors that have occurred.\~
	 *
	 * @since 2012-01-19
	 * @access protected
	 * @return string
	 */
	protected function setError(){
		$argumentos = func_get_args();
		if(empty($argumentos)) return(false);

		$clave = array_shift($argumentos);

		$this->aError[] = $clave.' ['.$this->microtimeSeg().'] '.vsprintf($this->aErrorMensajes[$clave], $argumentos);
		//trigger_error($this->aError[count($this->aError)-1]);

		return(true);
	}

	/**
	 * \~spanish Cambia mensajes de error (su texto).
	 * Util, por ejemplo, para traducciones.
	 * Si clave igual a false se reinicializan los mensajes.
	 * \~english Changes error messages.\~
	 *
	 * @access public
	 * @param mixed $clave \~spanish Clave del mensaje de error, o un array con todos los mensajes de error (array('clave' => 'mensaje de error',[...]);); si no existe se genera un error\~english Error message key or messages array\~
	 * @param string $valor \~spanish Nuevo texto para el mensaje de error\~english New error text\~
	 * @return boolean
	 */
	public function setErrorMensaje($clave = false, $valor = false){
		if($clave === false){ //reinicio de mensajes
			//TODO se pierden los mensajes iniciales de las clases herederas
			$this->aErrorMensajes = array(//'setErrorMensaje' => 'Se ha solicitado una clave desconocida de la lista de mensajes de error.',
			'tipoQuery1' => 'sqlazo::tipoQuery() Tipo de consulta no soportada: %s',
			'valoresEnQuery1' => 'sqlazo::valoresEnQuery() Tipo de parametro desconocido: %%%s',
			'setDDBBPuerto' => 'sqlazo::setDDBBPuerto() El puerto de la propiedad "sDDBBServidor" [%s] no es el mismo que el proporcionado [%s]; se utilizará %s',
			'operaQuery1' => 'sqlazo::operaQuery() No se puede acceder a la clase de parseo de consultas.',
			'operaQuery2' => 'sqlazo::operaQuery() Sólo se puede calcular el valor máximo para consultas tipo "SELECT".',
			'operaQuery3' => 'sqlazo::operaQuery() Error en la consulta tipo "max"; consulta:"%s".'
			);
			return(true);
		}

		$aMensajes = array();

		if(!is_array($clave)) $aMensajes[$clave] = $valor;
		else $aMensajes = $clave;

		foreach($aMensajes as $c => $v){
			//TODO metido en un switch obliga a reescribirlo cada vez que se cambie una clave de error en una clase heredera
			/*switch($c){
				//case 'otroError': //toda la lista sin break, menos default
				case 'conectar1': //class.sqlazo_mysql.inc //class.sqlazo_mysqli.inc
				case 'conectar2': //class.sqlazo_mysql.inc //class.sqlazo_mysqli.inc
				case 'desconectar1': //class.sqlazo_mysql.inc //class.sqlazo_mysqli.inc
				case 'seleccionarDDBB1': //class.sqlazo_mysql.inc //class.sqlazo_mysqli.inc
				case 'seleccionarDDBB2': //class.sqlazo_mysql.inc //class.sqlazo_mysqli.inc
				case 'query': //class.sqlazo_mysql.inc //class.sqlazo_mysqli.inc
				case 'aseguraQuery1': //class.sqlazo_mysql.inc //class.sqlazo_mysqli.inc
				case 'aseguraQuery2': //class.sqlazo_mysql.inc //class.sqlazo_mysqli.inc
				case 'aseguraQuery3': //class.sqlazo_mysql.inc //class.sqlazo_mysqli.inc
				case 'consulta1': //class.sqlazo_mysql.inc //class.sqlazo_mysqli.inc
				case 'consulta2': //class.sqlazo_mysql.inc //class.sqlazo_mysqli.inc
				case 'numResultados1': //class.sqlazo_mysql.inc //class.sqlazo_mysqli.inc
				case 'recPuntero1': //class.sqlazo_mysql.inc //class.sqlazo_mysqli.inc
				case 'leeFila1': //class.sqlazo_mysql.inc //class.sqlazo_mysqli.inc
				case 'leeFila2': //class.sqlazo_mysql.inc //class.sqlazo_mysqli.inc
				case 'leeFilasTodos1': //class.sqlazo_mysql.inc //class.sqlazo_mysqli.inc
				case 'leeFilasTodos2': //class.sqlazo_mysql.inc //class.sqlazo_mysqli.inc
				case 'setErrorMensaje': //class.sqlazo.inc
				case 'tipoQuery': //class.sqlazo_mysql.inc //class.sqlazo_mysqli.inc
				case 'tipoQuery1': //class.sqlazo.inc*/
					$this->aErrorMensajes[$c] = $v;
					/*break;
				default:
					$this->setError(__METHOD__);
					return(false);
			}*/
		}
		return(true);
	}

	/**
	 * \~spanish Constructor
	 * Los parametros por defecto conectan a un servidor local,
	 * con el usuario administrador (sin clave) y a la base de datos "test"
	 * \~english Constructor\~
	 */
	function sqlazo($servidor = 'localhost', $usuario = 'root', $password = '', $ddbb = 'test', $puerto = ''){
		$this->aError = false;
		$this->setErrorMensaje(); //inicializacion de los mensajes de error

		$this->bConectado = false;
		//$this->sMotor = 'mysql';
		$this->sDDBBServidor = $servidor;
		$this->setDDBBPuerto($puerto);
		$this->sDDBBUsuario = $usuario;
		$this->sDDBBPassword = $password;
		$this->sDDBB = $ddbb;
		$this->aResultados = array();
		$this->aFila = array();
		$this->aFilaTodos = array();
		$this->sTipoQuery = '';
		$this->sSubTipoQuery = '';
		$this->sPrefijo = '';
		$this->sPrefijoId = '_p_';
		$this->aTablas = array();
		$this->iTotalFilas = 0;
		$this->sUltimaId = false;
		$this->aTiempos = array();
		$this->sTagsPermitidosIns = 'todos';
		$this->sTagsPermitidosSel = 'todos';
	}

	/**
	 * \~spanish Tipo de query (primera palabra de la sentencia SQL).
	 * Sin incluir parentesis y demas (caso de UNION)
	 * \~english Query type.\~
	 *
	 * @todo detectar (guardando en array) todas las consultas que se hacen (por ejemplo en UNION) y las tablas utilizadas, categorizando segun sean JOIN ...; devuelve nombres imcompletos de tablas cuando llevan espacios
	 * @access protected
	 * @param string $query Query SQL
	 * @return boolean
	 */
	protected function tipoQuery($query = false){
		if($query === false) $query = $this->sQuery;
		//$aQuery = explode(' ', trim(str_replace(array('(', ')', "\n", "\r", "\t"), '', $query))); //evita que en UNION o similar (pueden empezar con parentesis) no encuentre el primer SELECT, saltos de carro indeseados, etc
		$aQuery = explode(' ', trim(preg_replace('/[\s|\(|\)]{2,}/', ' ', $query))); //evita que en UNION o similar (pueden empezar con parentesis) no encuentre el primer SELECT, saltos de carro indeseados, etc
		$this->sTipoQuery = strtolower($aQuery[0]);

		//que tablas estan implicadas en la consulta
		//TODO ver "mysql_field_table"
		$this->aTablas = array();
		$aPatron = array();
		switch($this->sTipoQuery){
			case 'alter':
			case 'create': //TODO revisar coincidencias y como se guarda el tipo de create y la tabla o base de datos que se crea
			case 'drop':
				$this->sSubTipoQuery = strtolower($aQuery[1]);
				$aPatron[] = '/'.$this->sTipoQuery.'\W+\w*\W+(?:IF.+EXISTS)*\W*([\w\.]*)/i';
				$aPatron[] = '/CONSTRAINT\W+([\w\.]*)/i';
				$aPatron[] = '/REFERENCES\W+([\w\.]*)/i';
				break;
			case 'describe':
			case 'explain':
			case 'show':
				if(strpos(strtolower($query), 'select ') === false){ //TODO debe contemplar otros tipos de consulta, como "EXPLAIN UPDATE..."
					$aPatron[] = '/'.$this->sTipoQuery.'\W+([\w\.]*)/i';
					break;
				}
			case 'select':
				$aPatron[] = '/FROM\W+([\w\.]*)/i'; //TODO no contempla viejo estilo de join's (select * from tabla1 t1, tabla2 t2 where t1.id1=t2.id1)
				$aPatron[] = '/JOIN\W+([\w\.]*)/i';
				break;
			case 'lock':
				$aPatron[] = '/TABLES\W+([\w\.]*)/i'; //TODO puede cerrar mas de una tabla en la misma instruccion
				break;
			case 'update':
			case 'insert':
				$aPatron[] = '/'.$this->sTipoQuery.'\W+([\w\.]*)/i';
				if(strpos(strtolower($query), 'into ') !== false){
					$aPatron = array();
					$aPatron[] = '/INTO\W+([\w\.]*)/i';
				}
			case 'delete':
				$aPatron[] = '/FROM\W+([\w\.]*)/i'; //TODO no contempla viejo estilo de join's (select * from tabla1 t1, tabla2 t2 where t1.id1=t2.id1)
				$aPatron[] = '/JOIN\W+([\w\.]*)/i';
				break;
			case 'commit':
			case 'rollback':
			case 'set':
			case 'start': //transaction
			case 'unlock':
			case 'use':
				break;
			default:
				$this->setError(__METHOD__.'1', $this->sTipoQuery);
				return(false);
		}

		$this->sQuery = $query;

		//reemplaza prefijo donde sea necesario
		foreach($aPatron as $sPat){
			if(preg_match_all($sPat, $query, $regs, PREG_PATTERN_ORDER)){
				for($cont=0;$cont<count($regs[0]);$cont++){
					$sTemp = '';
					if(strpos($regs[1][$cont], '.') === false) $sTemp = str_replace($this->sPrefijoId, $this->sPrefijo, $regs[1][$cont]);
					else $sTemp = str_replace($this->sPrefijoId, $this->sPrefijo, substr($regs[1][$cont], strpos($regs[1][$cont], '.')+1));
					$this->aTablas[$sTemp] = $sTemp;

					$this->sQuery = str_replace($regs[0][$cont], str_replace($regs[1][$cont], str_replace($this->sPrefijoId, $this->sPrefijo, $regs[1][$cont]), $regs[0][$cont]), $this->sQuery); //prefijado de tablas en la consulta
				}

				$this->aTablas = array_keys($this->aTablas); //elimina entradas duplicadas
			}
		}
		return(true);
	}

	/**
	 * NO FUNCIONA, "get_defined_vars" solo se llena con las variables del entorno en que es llamado
	 * Devuelve si una variable esta declarada.
	 * "isset" devuelve falso tanto si no esta declarada como si esta declarada a NULL; y lo mismo "is_null"
	 *
	 * @since 2012-03-13
	 * @access private
	 * @param string $var Nombre de la variable a comprobar
	 * @return boolean
	 */
	/*private function var_exists($var){
		$var = strpos($var, '$') === 0?substr($var, 1):$var; //si lleva el $ inicial, se le quita

		$aVars = get_defined_vars();
		return(key_exists($var, $aVars));
	}*/

	/**
	 * \~spanish Sustituye los valores indicados en la query por los parametros pasados.
	 * Imitacion de "vsprintf".
	 * \~english Replaces placeholder with their values.\~
	 *
	 * @access protected
	 * @param string $query Query SQL
	 * @param array $pars \~spanish Parametros de la query\~english Query parameters\~
	 * @return string
	 */
	protected function valoresEnQuery($query, $pars){
		//extrae los codigos de parametros de la SQL
		$query = str_replace('%%', "\x0b", $query); //evita que se corte la cadena por sitios incorrectos (reemplazos por tabulacion vertical)
		$aParams = explode('%', $query);
		$query = array_shift($aParams);

		foreach($aParams as $clave => $valor){
			$aVars = get_defined_vars(); //comprueba si existe la varialbe, con isset devuelve que no existe si esta declarada a NULL
			//$var_exists = key_exists('pars['.$clave.']', $aVars);

			switch(substr($valor, 0, 1)){ //el primer caracter es el tipo de dato, o %
				case '%':
					$query .= '%'.$valor;
					break;
				case 's': //string
				case 'd': //entero con signo
				case 'u': //entero sin signo
				case 'F': //flotante
				case 'f': //flotante
					//if(isset($pars[$clave]) && is_null($pars[$clave])){ //corrige la query para que acepte nulos
					if(isset($aVars['pars']) && is_array($aVars['pars']) && key_exists($clave, $aVars['pars']) && is_null($pars[$clave])){ //comprueba si se ha declarado la variable $pars[$clave] como NULL
						if(substr($query, strlen($query)-1, 1) == "'" || substr($query, strlen($query)-1, 1) == "\x0b"){ //limpia este final de cadena para poner el nulo
							$query = rtrim($query, "'%\x0b").'NULL';
						}
						else $query .= 'NULL';
						if(substr($valor, 1, 1) == "'" || substr($valor, 1, 1) == "\x0b"){ //limpia este principio de cadena para poner el nulo
							$query .= ltrim(substr($valor, 1), "'%\x0b");
						}
						else $query .= substr($valor, 1);
					}
					else{
						if(isset($pars[$clave])) $query .= $pars[$clave].substr($valor, 1);
						else{
							//$this->setError(__METHOD__.'1', $clave);
							return(false);
						}
					}
					break;
				default:
					$this->setError(__METHOD__.'1', substr($valor, 0, 1));
					return(false);
			}
		}
		if(isset($valor) && substr($valor, 0, 1) != "%") $aParams[] = substr($valor, 0, 1);

		$query = str_replace("\x0b", '%', $query); //recupera los comodines
		return($query);
	}

}