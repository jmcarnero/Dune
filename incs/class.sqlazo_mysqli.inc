<?php
/*Copyright (C) 2009 José M. Carnero <jm_carnero@sargazos.net>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
http://www.gnu.org/copyleft/gpl.html*/

//Constante con el directorio donde se encuentran las clases derivadas y/o dependientes de esta
defined('CLASSES_DIR') or define('CLASSES_DIR', dirname(__FILE__));

/**
 * Clase para conexion y querys a bases de datos MySQL >= 4.1.3
 * (MySQL Improved Extension)
 * #core PHP 5
 * Ejemplo de uso:
 * (es preferible usar el metodo descrito en "class.sqlazo.inc")
 * <code>
include_once("./inc/class.sqlazo.inc");
include_once("./inc/class.sqlazo_trazabilidad.inc");
include_once("./inc/class.sqlazo_mysqli.inc");

$oTest = new SQLazo_mysqli(DB_SERVER, DB_USER, DB_PASSWORD, DB_DATABASE);
//$oTest->conectar(DB_SERVER, DB_USER, DB_PASSWORD, DB_DATABASE); //se puede omitir si se han pasado los parametros de conexion al constructor

$oTest->consulta("SELECT * FROM _p_usuarios");

while($oTest->leeFila()){
	echo($oTest->aFila['login']."\n");
}

//var_dump($oTest->aTiempos);

$oTest->desconectar();
 * </code>
 *
 * @see class.sqlazo.inc,class.sqlazo_trazabilidad.inc
 * @author José M. Carnero
 * @version 2.5
 * @license http://www.gnu.org/copyleft/gpl.html
 * @package SQLazo
 */
class SQLazo_mysqli extends SQLazo_trazabilidad {

	/**
	 * Estado de seleccion de base de datos (true, false)
	 *
	 * @var boolean
	 * @access private
	 */
	private $bSeleccion;

	/**
	 * Mysqli_result, resultante de $this->oMysqli->store_result()
	 *
	 * @var object
	 * @access private
	 */
	private $oResultados;

	/**
	 * Objeto (primitivo) Mysqli
	 *
	 * @var object
	 * @access private
	 */
	private $oMysqli;

	/**
	 * Codificacion en que se lanzaran las querys; a vacio usara la que tenga la base de datos; valores: utf-8,
	 *
	 * @var string
	 */
	public $sCharset;

	/**
	 * Collation en que se lanzaran las querys; vacio usara la que tenga la base de datos; util para evitar problemas en las comparaciones de cadenas de distintas tablas
	 *
	 * @var string
	 */
	public $sCollation;

	/**
	 * Define que tipo de array devolvera una query: dos (MYSQL_BOTH | MYSQLI_BOTH) -> ambos tipos de array, aso (MYSQL_ASSOC | MYSQLI_ASSOC) -> asociativo, num (MYSQL_NUM | MYSQLI_NUM) -> numerico; por defecto (si no se define explicitamente valor): ambos; es valido asignandole las constantes entre parentesis (naturales de PHP)
	 *
	 * @var string
	 */
	public $sQueryTipoArray;

	/**
	 * Constructor
	 *
	 * Los parametros por defecto conectan a un servidor de MYSQL local,
	 * con el usuario administrador (sin clave) y a la base de datos "test"
	 * (que existe por defecto)
	 *
	 * @param string $servidor Servidor
	 * @param string $usuario Usuario
	 * @param string $password Clave
	 * @param string $ddbb Base de datos
	 * @param integer $puerto Puerto
	 */
	public function __construct($servidor = 'localhost', $usuario = 'root', $password = '', $ddbb = 'test', $puerto = 3306){
		parent::SQLazo_trazabilidad($servidor, $usuario, $password, $ddbb, $puerto);
		//$this->oMysqli = mysqli_init(); //nueva instancia de la clase mysqli (propia de PHP)
		//$this->oMysqli = & new mysqli(); //nueva instancia de la clase mysqli (propia de PHP) // new por referencia "deprecated" en php 5.3

		//comprobar que no da (o similar): Warning: mysqli::mysqli(): Headers and client library minor version mismatch. Headers:50149 Library:50210 in /home/jmanuel/archincloud/includes/class.ddbb_mysqli.inc on line 113
		$this->oMysqli = @ new mysqli(); //nueva instancia de la clase mysqli (propia de PHP)

		$this->oResultados = false;
		//$this->oResultados = & new mysqli_result(); //nueva instancia de la clase mysqli_result (propia de PHP)

		$this->sMotor = 'mysqli';
		$this->sQueryTipoArray = 'aso';

		$this->sCharset = $this->codificacion('charset');
		$this->sCollation = $this->codificacion('collation');

		$this->setErrorMensaje(array(
		'SQLazo_mysqli::aseguraQuery1' => 'Numero de parametros incorrecto',
		'SQLazo_mysqli::aseguraQuery2' => 'No corresponde el tipo de parametro (%%%s (num. %s)) con el valor proporcionado (%s)',
		'SQLazo_mysqli::aseguraQuery3' => 'Tipo de parametro desconocido: %%%s',
		'SQLazo_mysqli::conectar1' => 'No se puede conectar a base de datos. Extension MYSQL para PHP no instalada.',
		'SQLazo_mysqli::conectar2' => 'No se puede conectar a la base de datos <em>%s</em>, error: [%s] %s',
		'SQLazo_mysqli::conectar3' => 'No se puede crear una instancia del objeto Mysqli.',
		'SQLazo_mysqli::consulta1' => 'La consulta <em>%s</em> ha generado el error: [%s] %s',
		'SQLazo_mysqli::consulta2' => 'Tipo de consulta no soportada: %s',
		'SQLazo_mysqli::consultaInterna' => 'Error ([%s] %s) en la consulta interna: %s',
		'SQLazo_mysqli::desconectar1' => 'No se puede cerrar la conexion a la base de datos <em>%s</em>, error: [%s] %s',
		'SQLazo_mysqli::leeFila1' => 'Ning&uacute;n resultado a devolver.',
		'SQLazo_mysqli::leeFila2' => 'Intenta obtener un array de resultados de una consulta no "SELECT"',
		'SQLazo_mysqli::leeFilasTodos1' => 'Ning&uacute;n resultado a devolver.',
		'SQLazo_mysqli::leeFilasTodos2' => 'Intenta obtener un array de resultados de una consulta no "SELECT"',
		'SQLazo_mysqli::numResultados1' => 'No es posible contar el numero de registros, no se reconoce el tipo de query: %s',
		'SQLazo_mysqli::query' => 'Numero de parametros incorrecto',
		'SQLazo_mysqli::recPuntero1' => 'No se ha podido recolocar el puntero debido al error: [%s] %s',
		'SQLazo_mysqli::seleccionarDDBB1' => 'No se puede seleccionar la base de datos <em>%s</em>, error: [%s] %s',
		'SQLazo_mysqli::setCodificacion' => 'Error al cargar el juego de caracteres %s: %s',
		//'SQLazo_mysqli::setCodificacion' => 'No se pudo cambiar el juego de caracteres usado en la conexión',
		'SQLazo_mysqli::tipoArrayQuery' => 'El tipo de array (<em>%s</em>) que debe devolverse es desconocido',
		'SQLazo_mysqli::transaccionFinal' => 'No se ha completado correctamente la transacción.',
		'SQLazo_mysqli::transaccionInicio' => 'No se ha podido iniciar la transacción.'
		));
	}

	/**
	 * Comprueba que los tipos de parametros proporcionados correspondan y los limpia si no (evita inyeccion sql)
	 *
	 * @param string $query Query SQL
	 * @param array $pars Parametros de la query
	 * @return void
	 */
	protected function aseguraQuery($query, $pars){
		//extrae los codigos de parametros de la SQL
		$query = str_replace('%%', '', $query);
		$aParams = explode('%', $query);
		array_shift($aParams);
		$aTemp = $aParams;
		$aParams = array();

		foreach($aTemp as $valor){
			if(substr($valor, 0, 1) != "%") $aParams[] = substr($valor, 0, 1);
		}

		if(!is_array($pars)) $pars = array($pars); //se ha pasado un solo parametro y no como array
		if(count($aParams) != count($pars)){
			$this->setError(__METHOD__.'1');
			return false;
		}

		$i = 0;
		foreach($pars as $clave => $valor){
			switch(strtolower($aParams[$i])){ //los indices del array $aParams y $pars no siempre coinciden
				case 'd': //entero con signo
				case 'f': //flotante (F)
				case 'u': //entero sin signo
					if(!is_numeric($valor) || is_null($valor)){
						if(is_null($valor) || $valor == ''){
							$pars[$clave] = null;
						}
						else{
							$this->setError(__METHOD__ . '2', $aParams[$i], ($i + 1), (is_null($valor) ? 'null' : $valor));
						}
						//return(false);
					}
					break;
				case 's': //cadena de caracteres
					//cambia la codificacion de caracteres si no corresponde con la necesaria para lanzar la consulta
					//TODO comprobar y convertir mas juegos de caracteres; tantos como soporte mysql
					//if($this->sCharset == 'utf8') $valor = mb_detect_encoding($valor, 'UTF-8', true) == 'UTF-8' ? $valor : utf8_encode($valor);
					if(isset($valor)){
						$encoding = mb_detect_encoding($valor, mb_list_encodings(), true);
						if($encoding !== 'UTF-8') $valor = mb_convert_encoding($valor, 'UTF-8', $encoding);
					}

					//TODO ambas opciones son vulnerables a inyeccion SQL (addslashes mas), "prepared statements" es mas seguro
					if(function_exists('mysqli_real_escape_string')){ //PHP >= 4.3
						if(isset($valor)){
							if(get_magic_quotes_gpc()) $valor = stripslashes($valor);
							$pars[$clave] = $this->oMysqli->real_escape_string($valor);
						}
						else $pars[$clave] = null;
					}
					else{
						if(!get_magic_quotes_gpc())
							$pars[$clave] = isset($valor) ? addslashes($valor) : null; //escapado generico
					}
					break;
				default:
					$this->setError(__METHOD__.'3', $aParams[$i]);
					return false;
			}
			$i++;
		}

		return($pars);
	}

	/**
	 * Conexion al servidor.
	 * Selecciona automaticamente la base de datos,
	 * llamar a "seleccionarDDBB()" cambiando la propiedad "$this->sDDBB"
	 * para usar otra base de datos con esta misma conexion.
	 *
	 * @param string $servidor Servidor
	 * @param string $usuario Usuario
	 * @param string $clave Clave
	 * @param string $db Schema (base de datos)
	 * @param integer $puerto Puerto
	 * @return boolean
	 */
	public function conectar($servidor = false, $usuario = false, $clave = false, $db = false, $puerto = false){
		if(!class_exists('mysqli')){
			$this->setError(__METHOD__.'1');
			$this->bConectado = false;
			return false;
		}

		SQLazo::instCount(); //contador de instancias
		if($servidor) $this->sDDBBServidor = $servidor;
		if($usuario) $this->sDDBBUsuario = $usuario;
		if($clave) $this->sDDBBPassword = $clave;
		$this->setDDBBPuerto($puerto);

		//medicion de tiempo
		$iTiempoIni = $this->microtimeSeg();

		if(!$this->bConectado){
			//$this->oMysqli = mysqli_init(); //nueva instancia de la clase mysqli (propia de PHP)
			$this->oMysqli->init(); //inicializar (crea un recurso necesario para metodos como mysqli::real_connect)
		}

		if($this->sDDBBPuerto)
			$this->bConectado = @ $this->oMysqli->real_connect($this->sDDBBServidor, $this->sDDBBUsuario, $this->sDDBBPassword, false, $this->sDDBBPuerto);
		else
			$this->bConectado = @ $this->oMysqli->real_connect($this->sDDBBServidor, $this->sDDBBUsuario, $this->sDDBBPassword, false);

		$iTiempoFin = $this->microtimeSeg();
		$this->aTiempos['Conectar'] = number_format($iTiempoFin - $iTiempoIni, 6);
		$this->aTiempos['total'] = $iTiempoFin - $iTiempoIni; //se inicia el recuento del total de tiempo

		//PHP >= 5.2.9
		if(@ $this->oMysqli->connect_error){
			$this->setError(__METHOD__.'2', $this->sDDBB, @ $this->oMysqli->connect_errno, @ $this->oMysqli->connect_error);
			return false;
		}
		//PHP < 5.2.9
		elseif(mysqli_connect_error()){
			$this->setError(__METHOD__.'2', $this->sDDBB, mysqli_connect_errno(), mysqli_connect_error());
			return false;
		}
		elseif(!$this->bConectado){
			$this->setError(__METHOD__.'3');
			return false;
		}

		if($db === false)
			return true;
		else
			return $this->seleccionarDDBB($db);
	}

	/**
	 * Consulta a la base de datos, (si "SELECT", necesita de "SQLazo::leeFila()" para empezar a devolver resultados).
	 *
	 * @param string $query Query SQL
	 * @param array $pars Parametros de la query
	 * @param integer $cantidad Numero de registros a devolver
	 * @param integer $inicial Registro inicial a devolver
	 * @return boolean
	 * @abstract
	 */
	public function consulta($query = false, $pars = array(), $cantidad = 0, $inicial = 0){
		$iRet = true;

		if(!$this->bConectado){
			if(!$this->conectar($this->sDDBBServidor, $this->sDDBBUsuario, $this->sDDBBPassword, $this->sDDBB))
				return false;
		}

		if($query)
			$this->query($query, $pars, $cantidad, $inicial);

		//limpia los arrays de resultados, para evitar "datos fantasma"
		$this->aFila = array();
		$this->aFilaTodos = array();

		//medicion de tiempo
		$iTiempoIni = $this->microtimeSeg();

		/*si se va a recuperar gran cantidad de datos con mysqli::query se recomienda usar MYSQLI_USE_RESULT (por defecto MYSQLI_STORE_RESULT)
		ej: $mysqli->query("SELECT * FROM datos", MYSQLI_USE_RESULT)
		pero esto impedira lanzar funciones que interactuen con el servidor (devolviendo error 'out of sync'), como otra query*/
		/*nota: mysqli::use_result recupera sin colocar en bufer (unbuffered) las filas de resultado, mas rapido pero bloquea las tablas implicadas y da problemas con el recuento de resultados*/

		if(!empty($this->oResultados->num_rows)){
			$this->oResultados->free(); //importante cuando se usa mysqli::store_result
		}

		if($this->bTrazabilidad){
			$this->transaccionInicio(); //se inicia el guardado de trazas
		}

		$sTempUltIns = 0; //id de la ultima insercion, se guarda temporalmente para evitar que la pise el fin de la transaccion
		if($this->sCharset) //codificacion en que se lanzan las consultas //si se lanza despues de mysqli::init casca
			$this->oMysqli->set_charset($this->sCharset);

		if($this->oMysqli->real_query($this->sQuery)){
			if($this->sTipoQuery == 'insert'){
				$sTempUltIns = $this->oMysqli->insert_id;
			}
			else{
				$this->oResultados = $this->oMysqli->store_result(); //recurso de resultados
			}
		}
		else{
			$this->setError(__METHOD__.'1', $this->sQuery, $this->oMysqli->errno, $this->oMysqli->error);

			if($cantidad){
				$this->sQuery = str_replace('SQL_CALC_FOUND_ROWS ', '', $this->sQuery); //no es necesario en caso de error y no es algo puesto por el usuario
			}

			if($this->bTrazabilidad)
				$this->transaccionFinal(); //finaliza el guardado de trazas

			return false;
		}

		//seleccionar una base de datos despues de crearla
		if($this->sTipoQuery == 'create' && $this->sSubTipoQuery == 'database')
			$this->seleccionarDDBB($this->aTablas[0]); //despues de crear una base de datos la selecciona para las siguientes sentencias

		if($this->sTipoQuery == 'use')
			$this->seleccionarDDBB($this->aTablas[0]); //use `database`

		$iTiempoFin = $this->microtimeSeg();
		$this->aTiempos[$this->sTipoQuery] = number_format($iTiempoFin - $iTiempoIni, 6);
		$this->aTiempos['total'] += ($iTiempoFin - $iTiempoIni);

		switch($this->sTipoQuery){
			case 'describe':
			case 'explain':
			case 'select':
			case 'show':
				if($cantidad){
					//mysqli::num_rows
					//medicion de tiempo
					$iTiempoIni = $this->microtimeSeg();

					//calculo de resultados para consultas con LIMIT
					$aResultados = $this->consultaInterna('SELECT FOUND_ROWS() AS totalRows');
					/*$this->oMysqli->real_query('SELECT FOUND_ROWS() AS totalRows');
					$oResultados = $this->oMysqli->use_result();
					$aResultados = $oResultados->fetch_array();*/

					$this->iTotalFilas = (int) $aResultados[0]['totalRows'];
					$this->sQuery = str_replace('SQL_CALC_FOUND_ROWS ', '', $this->sQuery); //no es necesario despues de calcular el total de filas y no es algo puesto por el usuario

					//$oResultados->close();

					$iTiempoFin = $this->microtimeSeg();
					$this->aTiempos['totalLimit'] = number_format($iTiempoFin - $iTiempoIni, 6);
					$this->aTiempos['total'] += ($iTiempoFin - $iTiempoIni);
				}
				else
					$this->numResultados(); //calculo del total de resultados

				$this->listaCampos(); //lista de campos de la consulta
				break;
			case 'insert':
				$this->sUltimaId = $sTempUltIns; //-> devuelve la ultima id insertada
			case 'alter':
			case 'create':
			case 'delete':
			case 'drop':
			case 'use':
			case 'update':
				$this->numResultados(); //calculo del total de resultados
				break;
			case 'commit':
			case 'lock':
			case 'rollback':
			case 'set':
			case 'start': //transaction
			case 'unlock':
				break;
			default:
				$this->setError(__METHOD__.'2', $this->sTipoQuery);
				$iRet = false;
		}

		if($this->bTrazabilidad)
			$this->transaccionFinal(); //finaliza el guardado de trazas; no puede estar antes del recuento, ya que este devolveria valores incorrectos al sustituir la traza a la que interesaba como ultima consulta

		return $iRet;
	}

	/**
	 * Metodo para consultas internas de la clase.
	 * Devuelve el array de resultados completo (como "SQLazo::leeFilasTodos()") o falso si ha habido error.
	 *
	 * @param string $sql Consulta SQL
	 * @return array
	 */
	protected function consultaInterna($sql){
		//TODO debe devolver al menos la id en caso de insert (tambien para udate/delete?)
		if(!$this->oMysqli->real_query($sql)){
			$this->setError(__METHOD__, $this->oMysqli->errno, $this->oMysqli->error, $sql);
			return false;
		}

		$oResultados = $this->oMysqli->use_result();
		$aResultados = array();

		if($oResultados){
			if(function_exists('mysqli_result::fetch_all'))
				$aResultados = $oResultados->fetch_all();
			else{
				while($aResultados[] = $oResultados->fetch_array());
			}

			$oResultados->close();
		}

		return $aResultados;
	}

	/**
	 * Desconexion del servidor.
	 *
	 * @return boolean
	 */
	public function desconectar(){
		if(!empty($this->oResultados))
			@ $this->oResultados->free(); //fuerza liberacion de memoria, solo sirve para "SELECT" (devuelve booleano) //TODO "Warning: mysqli_result::free() [mysqli-result.free]: Couldn't fetch mysqli_result" en ocasiones

		if(SQLazo::instCount(-1) == 0){
			//PHP >= 5.2.9
			if(!$this->oMysqli->close()){
				if(!$this->bSeleccion){
					$this->setError(__METHOD__.'1', $this->sDDBB, @ $this->oMysqli->connect_errno, @ $this->oMysqli->connect_error);
				}
				$this->bConectado = true;
				return false;
			}
			//PHP < 5.2.9
			/*elseif(!mysqli_close()){
				if(!$this->bSeleccion){
					$this->setError(__METHOD__.'1', $this->sDDBB, mysqli_connect_errno(), mysqli_connect_error());
				}
				$sReturn = false;
			}*/
			else
				$sReturn = true;
		}
		else
			$sReturn = true;

		$this->bSeleccion = false;
		$this->bConectado = false;
		return $sReturn;
	}

	/**
	 * Devuelve las banderas de los campos.
	 * Tomada de phpMyAdmin.
	 *
	 * @param integer $i field
	 * @return string Field flags
	 */
	private function fieldFlags($i){
		// This is missing from PHP 5.2.5, see http://bugs.php.net/bug.php?id=44846
		if(!defined('MYSQLI_ENUM_FLAG')){
			define('MYSQLI_ENUM_FLAG', 256); // see MySQL source include/mysql_com.h
		}

		$f = $this->oResultados->fetch_field_direct($i);
		$type = $f->type;
		$charsetnr = $f->charsetnr;
		$f = $f->flags;
		$flags = '';

		if($f & MYSQLI_UNIQUE_KEY_FLAG)     { $flags .= 'unique ';}
		if($f & MYSQLI_NUM_FLAG)            { $flags .= 'num ';}
		if($f & MYSQLI_PART_KEY_FLAG)       { $flags .= 'part_key ';}
		if($f & MYSQLI_SET_FLAG)            { $flags .= 'set ';}
		if($f & MYSQLI_TIMESTAMP_FLAG)      { $flags .= 'timestamp ';}
		if($f & MYSQLI_AUTO_INCREMENT_FLAG) { $flags .= 'auto_increment ';}
		if($f & MYSQLI_ENUM_FLAG)           { $flags .= 'enum ';}
		// See http://dev.mysql.com/doc/refman/6.0/en/c-api-datatypes.html:
		// to determine if a string is binary, we should not use MYSQLI_BINARY_FLAG
		// but instead the charsetnr member of the MYSQL_FIELD
		// structure. Watch out: some types like DATE returns 63 in charsetnr
		// so we have to check also the type.
		// Unfortunately there is no equivalent in the mysql extension.
		if(($type == MYSQLI_TYPE_TINY_BLOB || $type == MYSQLI_TYPE_BLOB || $type == MYSQLI_TYPE_MEDIUM_BLOB || $type == MYSQLI_TYPE_LONG_BLOB || $type == MYSQLI_TYPE_VAR_STRING || $type == MYSQLI_TYPE_STRING) && 63 == $charsetnr)                { $flags .= 'binary ';}
		if($f & MYSQLI_ZEROFILL_FLAG)       { $flags .= 'zerofill ';}
		if($f & MYSQLI_UNSIGNED_FLAG)       { $flags .= 'unsigned ';}
		if($f & MYSQLI_BLOB_FLAG)           { $flags .= 'blob ';}
		if($f & MYSQLI_MULTIPLE_KEY_FLAG)   { $flags .= 'multiple_key ';}
		if($f & MYSQLI_UNIQUE_KEY_FLAG)     { $flags .= 'unique_key ';}
		if($f & MYSQLI_PRI_KEY_FLAG)        { $flags .= 'primary_key ';}
		if($f & MYSQLI_NOT_NULL_FLAG)       { $flags .= 'not_null ';}

		return trim($flags);
	}

	/**
	 * Lee una fila de resultados.
	 *
	 * @return boolean
	 */
	public function leeFila(){
		//no se pueden devolver filas si no las hay
		$sError = $this->getError();
		if(!empty($sError)){
			$this->setError(__METHOD__.'1');
			return false;
		}

		//este metodo solo tiene sentido con consultas tipo "SELECT"
		if($this->sTipoQuery == 'select' || $this->sTipoQuery == 'describe' || $this->sTipoQuery == 'explain' || $this->sTipoQuery == 'show'){

			if(!$this->tipoArrayQuery($this->sQueryTipoArray)) //tipo de array que devolvera la query
				return false;

			//medicion de tiempo
			$iTiempoIni = $this->microtimeSeg();

			$aTemp = $this->aFila; //conserva el ultimo array de resultados (la ultima fila), que sera pisada en el siguiente if si llega al final del recordset

			if($this->aFila = $this->oResultados->fetch_array($this->sQueryTipoArray)){
				$iTiempoFin = $this->microtimeSeg();
				$this->aTiempos['fila'][] = number_format($iTiempoFin - $iTiempoIni, 6);
				$this->aTiempos['total'] += ($iTiempoFin - $iTiempoIni);

				//elimina tags html y php de los resultados
				if(!empty($this->sTagsPermitidosSel) && $this->sTagsPermitidosSel != 'todos'){
					foreach($this->aFila as $key => $valor){
						$this->aFila[$key] = strip_tags($valor, $this->sTagsPermitidosSel);
					}
				}

				return true;
			}
			else{
				$this->aFila = $aTemp;
				return false;
			}
		}
		else{
			$this->setError(__METHOD__.'2');
			return false;
		}
	}

	/**
	 * Crea un array con todas las filas de resultados.
	 *
	 * @return boolean
	 */
	public function leeFilasTodos(){
		//no se pueden devolver filas si no las hay
		$sError = $this->getError();
		if(!empty($sError)){
			$this->setError(__METHOD__.'1');
			return false;
		}

		//este metodo solo tiene sentido con consultas tipo "SELECT"
		if($this->sTipoQuery == 'describe' || $this->sTipoQuery == 'explain' || $this->sTipoQuery == 'select' || $this->sTipoQuery == 'show'){
			//recoloca el puntero en el inicio para obtener todos los resultados
			if($this->iTotalFilas > 0) $this->recPuntero();

			if(!$this->tipoArrayQuery($this->sQueryTipoArray)) //tipo de array que devolvera la query
				return false;

			//medicion de tiempo
			$iTiempoIni = $this->microtimeSeg();

			if(function_exists('mysqli_result::fetch_all'))
				$this->aFilaTodos = $this->oResultados->fetch_all($this->sQueryTipoArray);
			else{
				while($this->aFilaTodos[] = $this->oResultados->fetch_array($this->sQueryTipoArray));

				if(empty($this->aFilaTodos[count($this->aFilaTodos) - 1])) //el anterior bucle (si todo es correcto) colocara un false en el ultimo elemento, sobra
					array_pop($this->aFilaTodos);
			}

			//elimina tags html y php de los resultados
			if(!empty($this->sTagsPermitidosSel) && $this->sTagsPermitidosSel != 'todos'){
				foreach($this->aFilaTodos as $clave => $valor){
					foreach($valor as $clave2 => $valor2){
						$this->aFilaTodos[$clave][$clave2] = strip_tags($valor2, $this->sTagsPermitidosSel);
					}
				}
			}

			$iTiempoFin = $this->microtimeSeg();
			$this->aTiempos['filasTodos'] = number_format($iTiempoFin - $iTiempoIni, 6);
			$this->aTiempos['total'] += ($iTiempoFin - $iTiempoIni);

			return true;
		}
		else{
			$this->setError(__METHOD__.'2');
			return false;
		}
	}

	/**
	 * Lista de los campos de la consulta
	 * Devuelve tanto por posicion en el recordset de resultados como por nombre de campo.
	 *
	 * @return boolean
	 */
	private function listaCampos(){
		if(!is_object($this->oResultados))
			return false; //no se puede devolver la lista de campos sin objeto de resultados //TODO devolver error?

		$this->aCampos = array(); //se vacia, por si se vuelve a llamar la funcion
		// Build an associative array for a type look up
		$typeAr = array();
		$typeAr[MYSQLI_TYPE_DECIMAL]     = 'real';
		$typeAr[MYSQLI_TYPE_NEWDECIMAL]  = 'real';
		$typeAr[MYSQLI_TYPE_BIT]         = 'int';
		$typeAr[MYSQLI_TYPE_TINY]        = 'int';
		$typeAr[MYSQLI_TYPE_SHORT]       = 'int';
		$typeAr[MYSQLI_TYPE_LONG]        = 'int';
		$typeAr[MYSQLI_TYPE_FLOAT]       = 'real';
		$typeAr[MYSQLI_TYPE_DOUBLE]      = 'real';
		$typeAr[MYSQLI_TYPE_NULL]        = 'null';
		$typeAr[MYSQLI_TYPE_TIMESTAMP]   = 'timestamp';
		$typeAr[MYSQLI_TYPE_LONGLONG]    = 'int';
		$typeAr[MYSQLI_TYPE_INT24]       = 'int';
		$typeAr[MYSQLI_TYPE_DATE]        = 'date';
		$typeAr[MYSQLI_TYPE_TIME]        = 'time';
		$typeAr[MYSQLI_TYPE_DATETIME]    = 'datetime';
		$typeAr[MYSQLI_TYPE_YEAR]        = 'year';
		$typeAr[MYSQLI_TYPE_NEWDATE]     = 'date';
		$typeAr[MYSQLI_TYPE_ENUM]        = 'unknown';
		$typeAr[MYSQLI_TYPE_SET]         = 'unknown';
		$typeAr[MYSQLI_TYPE_TINY_BLOB]   = 'blob';
		$typeAr[MYSQLI_TYPE_MEDIUM_BLOB] = 'blob';
		$typeAr[MYSQLI_TYPE_LONG_BLOB]   = 'blob';
		$typeAr[MYSQLI_TYPE_BLOB]        = 'blob';
		$typeAr[MYSQLI_TYPE_VAR_STRING]  = 'string';
		$typeAr[MYSQLI_TYPE_STRING]      = 'string';
		// MySQL returns MYSQLI_TYPE_STRING for CHAR
		// and MYSQLI_TYPE_CHAR === MYSQLI_TYPE_TINY
		// so this would override TINYINT and mark all TINYINT as string
		// https://sf.net/tracker/?func=detail&aid=1532111&group_id=23067&atid=377408
		//$typeAr[MYSQLI_TYPE_CHAR]        = 'string';
		$typeAr[MYSQLI_TYPE_GEOMETRY]    = 'unknown';
		$typeAr[MYSQLI_TYPE_BIT]         = 'bit';

		for($i = 0; $i < $this->oMysqli->field_count; $i++){
			$this->aCampos[$i]['nombre'] = $this->oResultados->fetch_field_direct($i)->name;
			$this->aCampos[$this->aCampos[$i]['nombre']]['pos'] = $i;
			$this->aCampos[$this->aCampos[$i]['nombre']]['tipo'] = $this->aCampos[$i]['tipo'] = $typeAr[$this->oResultados->fetch_field_direct($i)->type];
			$this->aCampos[$this->aCampos[$i]['nombre']]['long'] = $this->aCampos[$i]['long'] = $this->oResultados->fetch_field_direct($i)->length;
			$this->aCampos[$this->aCampos[$i]['nombre']]['flags'] = $this->aCampos[$i]['flags'] = $this->fieldFlags($i);
			// Enhance the field objects for mysql-extension compatibilty
			//$flags = explode(' ', $fields[$i]->flags);
			//array_unshift($flags, 'dummy');
			/*$this->aCampos[$i]->multiple_key = (int) (bool) ($fields[$i]->_flags & MYSQLI_MULTIPLE_KEY_FLAG);
			$this->aCampos[$i]->primary_key = (int) (bool) ($fields[$i]->_flags & MYSQLI_PRI_KEY_FLAG);
			$this->aCampos[$i]->unique_key = (int) (bool) ($fields[$i]->_flags & MYSQLI_UNIQUE_KEY_FLAG);
			$this->aCampos[$i]->not_null = (int) (bool) ($fields[$i]->_flags & MYSQLI_NOT_NULL_FLAG);
			$this->aCampos[$i]->unsigned = (int) (bool) ($fields[$i]->_flags & MYSQLI_UNSIGNED_FLAG);
			$this->aCampos[$i]->zerofill = (int) (bool) ($fields[$i]->_flags & MYSQLI_ZEROFILL_FLAG);
			$this->aCampos[$i]->numeric = (int) (bool) ($fields[$i]->_flags & MYSQLI_NUM_FLAG);
			$this->aCampos[$i]->blob = (int) (bool) ($fields[$i]->_flags & MYSQLI_BLOB_FLAG);*/
		}
		return true;
	}

	/**
	 * Calcular el numero de filas devueltas o afectadas por la query.
	 *
	 * @return boolean
	 */
	private function numResultados(){
		switch($this->sTipoQuery){
			case 'describe':
			case 'explain':
			case 'select':
			case 'show':
				//si el numero devuelto supera el rango de integer (segun maquina y sistema operativo) se devolvera como cadena
				$this->iTotalFilas = $this->oResultados->num_rows;
				return true;
				break;
			case 'alter':
			case 'create':
			case 'drop':
			case 'use':
			case 'delete':
			case 'insert':
			case 'update':
				//si el numero devuelto supera el rango de integer (segun maquina y sistema operativo) se devolvera como cadena
				//entero > 0 indica el numero de filas afectadas (o recuperadas, que no es el caso)
				//0 indica que un UPDATE no afecto a ninguna fila, ninguna fila coincide con un WHERE o que la query aun no ha sido ejecutada
				//-1 indica error
				$this->iTotalFilas = $this->oMysqli->affected_rows;

				if($this->iTotalFilas < 1)
					$this->iTotalFilas = false;

				return true;
				break;
			case 'commit':
			case 'lock':
			case 'rollback':
			case 'set':
			case 'start': //transaction
			case 'unlock':
				break;
			default:
				$this->setError(__METHOD__.'1', $this->sTipoQuery)."\n";
				return false;
		}
	}

	/**
	 * Construir query
	 *
	 * Si ya se ha asignado una query mediante la propiedad "sQuery" y no se desea cambiar NO llamar a esta funcion y llamar al metodo "consulta" sin parametros
	 * ej: $obj->fQuery("SELECT * FROM tabla WHERE campo=%s AND b=%s", array('1', '2'))
	 * cuando se requiera el comodin SQL "%" escribirlo doble "%%"; ej: $obj->consulta("SELECT * FROM tabla WHERE campo='%%%s%%'", array('hola')) ->producira la salida: "SELECT * FROM tabla WHERE campo='%hola%'"
	 *
	 * %s -> tomara del array de parametros una cadena de caracteres (en la misma posicion relativa)
	 * %d -> entero con signo
	 * %u -> entero sin signo
	 * %F -> flotante (PHP>4.3)
	 *
	 * @param string $query Query SQL
	 * @param array $pars Parametros de la query
	 * @param integer $cantidad Numero de registros a devolver
	 * @param integer $inicial Registro inicial a devolver
	 * @return void
	 */
	public function query($query, $pars = array(), $cantidad = 0, $inicial = 0){
		$inicial = is_numeric($inicial) ? $inicial : 0;
		$cantidad = is_numeric($cantidad) ? $cantidad : 0;

		//elimina tags html y php de los parametros de la consulta
		//si los parametros no van en el array "pars" no tiene efecto
		if($this->sTagsPermitidosIns != 'todos'){
			for($i = 0; $i < count($pars); $i++){
				if(isset($pars[$i])) $pars[$i] = strip_tags($pars[$i], $this->sTagsPermitidosIns);
			}
		}

		//preparar parametros para que no den problemas SQL
		//if(!get_magic_quotes_gpc()){
			//$pars = array_map('addslashes', $pars);
			if(count($pars)) $pars = $this->aseguraQuery($query, $pars);
		//}

		$this->tipoQuery($query); //comprueba el tipo de query

		//$this->sQuery = @vsprintf($this->sQuery, $pars);
		$this->sQuery = $this->valoresEnQuery($this->sQuery, $pars);
		if($this->sQuery === false){
			$this->setError(__METHOD__);
			return false;
		}

		//suma a la consulta "SQL_CALC_FOUND_ROWS", permite tener el total de filas en consultas limitadas con "SELECT FOUND_ROWS()", sin volver a lanzar la consulta
		if($cantidad){
			$iSelectPos = strpos(strtoupper($this->sQuery), 'SELECT ');
			if($iSelectPos !== false && strpos($this->sQuery, 'SQL_CALC_FOUND_ROWS') === false)
				$this->sQuery = substr($this->sQuery, 0, $iSelectPos + 7).'SQL_CALC_FOUND_ROWS '.substr($this->sQuery, $iSelectPos + 7);

			//si "$inicial=0" devuelve "$cantidad" de registros desde el primero
			//si se quieren devolver todos los que haya desde un "$inicial=x", dar como "$cantidad" un valor mayor al total de registros que pueda devolver la consulta (un numero al azar suficientemente grande, por ejemplo 123456789123456789123456789)
			$this->sQuery .= ' LIMIT '.$inicial.','.$cantidad;
		}
	}

	/**
	 * Recolocar puntero en el array de resultados (solo para SELECT)
	 *
	 * @param integer $pos Posicion puntero
	 * @return boolean
	 * @abstract
	 */
	public function recPuntero($pos = 0){
		//mysqli_result::data_seek solo puede usarse con resultados en buffer como los obtenidos con mysqli::store_result() o mysqli:query()
		if($this->iTotalFilas && !$this->oResultados->data_seek($pos)){
			$this->setError(__METHOD__.'1', $this->oMysqli->errno, $this->oMysqli->error);
			return false;
		}

		return true;
	}

	/**
	 * Seleccion de base de datos
	 *
	 * @param string $db Base de datos
	 * @return boolean
	 */
	public function seleccionarDDBB($db = false){
		if(!empty($db)){
			//if($this->sDDBB == $db) return(true); //sale con cierto si ya esta seleccionada la base de datos
			$this->sDDBB = $db;
		}
		elseif($this->bSeleccion)
			return true; //devuelve cierto si no se requiere seleccionar una base de datos concreta, como cuando se desee crear una

		//medicion de tiempo
		$iTiempoIni = $this->microtimeSeg();

		$this->bSeleccion = @ $this->oMysqli->select_db($this->sDDBB);

		$iTiempoFin = $this->microtimeSeg();
		$this->aTiempos['SeleccionarDDBB'] = number_format($iTiempoFin - $iTiempoIni, 6);
		$this->aTiempos['total'] += ($iTiempoFin - $iTiempoIni);

		if(!$this->bSeleccion){
			$this->setError(__METHOD__.'1', $this->sDDBB, @ $this->oMysqli->errno, @ $this->oMysqli->error);
		}

		//codificacion en que se lanzaran las consultas a la base de datos
		if(SQLazo::instCount(0) == 1 && $this->sCharset != '')
			$this->setCodificacion($this->sCharset, $this->sCollation);

		return $this->bSeleccion;
	}

	/**
	 * Cambia la codificacion con la que se lanzan las consultas a la base de datos.
	 * Devuelve falso si no se ha podido cambiar
	 *
	 * @since 2010-09-09
	 * @param string $charset Juego de caracteres, reconocible por MYSQL
	 * @param string $collation reconocible por MYSQL
	 * @return boolean
	 */
	public function setCodificacion($charset = '', $collation = ''){
		//TODO deberia detectarse en que codificacion funciona el servidor y adecuar la conversion a eso (si el usuario no solicita una explicitamente)

		//codificacion en que se lanzaran las consultas a la base de datos
		//TODO no soluciona error "illegal mix of collation"
		/*es equivalente a las tres sentencias:
			SET character_set_client = x;
			SET character_set_results = x;
			SET character_set_connection = x;*/
		/*estas muestran los valores de las anteriores
		SHOW VARIABLES LIKE 'character_set%';
		SHOW VARIABLES LIKE 'collation%';*/

		if(!$this->oMysqli->set_charset($charset)){
			$this->setError(__METHOD__, $charset, $this->oMysqli->error);
			return false;
		}
		/*else {
			printf("Current character set: %s\n", $this->oMysqli->character_set_name());
		}/*
		/*if($charset != ''){
			$this->oMysqli->set_charset($charset);
			/*if($collation != ''){
				if(!$this->consultaInterna("SET NAMES '$charset' COLLATE '$collation'")){
					$this->setError(__METHOD__);
					return(false);
				}
				else $this->consultaInterna("SET collation_connection = '".$collation."'");
			}
			else{
				if(!$this->consultaInterna("SET NAMES '$charset'")){
					$this->setError(__METHOD__);
					return(false);
				}
			}*/

			/*if(!$this->consultaInterna("SET CHARACTER SET '$this->sCharset'")){
				$this->setError('seleccionarDDBB2');
			}*/
		//}

		$this->sCharset = $charset;
		$this->sCollation = $collation;
		return true;
	}

	/**
	 * Tipo de array que devolvera la query.
	 *
	 * @param string $tipo Tipo de array que devolvera la consulta
	 * @return boolean
	 */
	private function tipoArrayQuery($tipo = ''){
		if(!empty($tipo))
			$this->sQueryTipoArray = $tipo;

		//tambien admite las constantes de mysql (no i)
		switch($this->sQueryTipoArray){
			case 'dos': //devuelve array asociativo y numerico
			case MYSQL_BOTH:
				$this->sQueryTipoArray = MYSQLI_BOTH;
			case MYSQLI_BOTH:
				break;
			case 'aso': //devuelve array asociativo
			case MYSQL_ASSOC:
				$this->sQueryTipoArray = MYSQLI_ASSOC;
			case MYSQLI_ASSOC:
				break;
			case 'num': //devuelve array numerico
			case MYSQL_NUM:
				$this->sQueryTipoArray = MYSQLI_NUM;
			case MYSQLI_NUM:
				break;
			default:
				$this->setError(__METHOD__, $this->sQueryTipoArray)."\n";
				return false;
		}

		return true;
	}

	/**
	 * Concluye la transaccion, pasos posteriores a la ejecucion de la consulta solicitada.
	 * Sobreescribir en la clase hija si interesa; por ejemplo "mysqli" tiene funciones propias para controlar transacciones.
	 * Nota: en mysql solo funcionara con tablas transaccionales (como innodb)
	 *
	 * @return boolean
	 */
	protected function transaccionFinal(){
		$bRet = true;

		$sError = $this->getError();
		if(empty($sError))
			$bRet = $this->oMysqli->commit(); //fija las querys
		else
			$bRet = $this->oMysqli->rollback(); //deshace las querys

		if(!$bRet || !$this->oMysqli->autocommit(true)){
			$this->setError(__METHOD__);
			return false;
		}

		if($this->bTrazabilidad) $this->guardaTraza(); //guardado de trazas

		return true;
	}

	/**
	 * Inicia la transaccion con la base de datos, pasos previos a lanzar la consulta solicitada.
	 * Sobreescribir en la clase hija si interesa; por ejemplo "mysqli" tiene funciones propias para controlar transacciones.
	 * Nota: en mysql solo funcionara con tablas transaccionales (como innodb)
	 *
	 * @return boolean
	 */
	protected function transaccionInicio(){
		if(!$this->oMysqli->autocommit(false)){
			$this->setError(__METHOD__);
			return false;
		}
		return true;
	}

}
