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
 * Capa de abstracción de conexión y consulta a bases de datos.
 * MYSQL
 *
 * @author José M. Carnero
 * @version 2.5
 * @license http://www.gnu.org/copyleft/gpl.html
 * @package Dune
 */

/**
 * Clase para conexion y querys a bases de datos.
 *
 * Para mysql -> MySQL 4, 5
 * #core PHP 4
 * Ejemplo de uso:
 * (es preferible usar el metodo descrito en "class.sqlazo.inc")
 * <code>
include_once("./inc/class.sqlazo.inc");
include_once("./inc/class.sqlazo_trazabilidad.inc");
include_once("./inc/class.sqlazo_mysql.inc");

$oTest = new sqlazo_mysql(DB_SERVER, DB_USER, DB_PASSWORD, DB_DATABASE);
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
 */
class sqlazo_mysql extends sqlazo_trazabilidad{

	/**
	 * Identificador de enlace
	 *
	 * @var boolean
	 * @access private
	 */
	var $dConexion;

	/**
	 * Estado de seleccion de base de datos (true, false)
	 *
	 * @var boolean
	 * @access private
	 */
	var $bSeleccion;

	/**
	 * Identificador de la consulta
	 *
	 * @var resource
	 * @access private
	 */
	var $rResultados;

	/**
	 * Codificacion en que se lanzaran las querys; a vacio usara la que tenga la base de datos; valores: utf-8,
	 *
	 * @var string
	 */
	var $sCharset;

	/**
	 * Collation en que se lanzaran las querys; vacio usara la que tenga la base de datos; util para evitar problemas en las comparaciones de cadenas de distintas tablas
	 *
	 * @var string
	 */
	var $sCollation;

	/**
	 * Define que tipo de array devolvera una query: dos (MYSQL_BOTH) -> ambos tipos de array, aso (MYSQL_ASSOC) -> asociativo, num (MYSQL_NUM) -> numerico; por defecto (si no se define explicitamente valor): ambos; es valido asignandole las constantes entre parentesis (naturales de PHP)
	 *
	 * @var string
	 */
	var $sQueryTipoArray;

	/**
	 * Comprueba que los tipos de parametros proporcionados correspondan y los limpia si no (evita inyeccion sql)
	 *
	 * @access protected
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
			return(false);
		}

		$i = 0;
		foreach($pars as $clave => $valor){
			switch(strtolower($aParams[$i])){ //los indices del array $aParams y $pars no siempre coinciden
				case 'd': //entero con signo
				case 'f': //flotante (F)
				case 'u': //entero sin signo
					if(!is_numeric($valor) || is_null($valor)){
						if(is_null($valor) || $valor == '')
							$pars[$clave] = NULL;
						else
							$this->setError(__METHOD__.'2', $aParams[$i], ($i+1), (is_null($valor)?'null':$valor));
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

					//TODO ambas opciones son vulnerables a inyeccion SQL (addslashes mas)
					if(function_exists('mysql_real_escape_string')){ //PHP >= 4.3
						if(isset($valor)){
							if(get_magic_quotes_gpc()) $valor = stripslashes($valor);
							$pars[$clave] = mysql_real_escape_string($valor, $this->dConexion);
						}
						else $pars[$clave] = NULL;
					}
					else{
						if(!get_magic_quotes_gpc()) $pars[$clave] = isset($valor)?addslashes($valor):NULL; //escapado generico
					}
					break;
				default:
					$this->setError(__METHOD__.'3', $aParams[$i]);
					return(false);
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
	 * @access public
	 * @param string $servidor Servidor
	 * @param string $usuario Usuario
	 * @param string $clave Clave
	 * @param string $db Schema (base de datos)
	 * @return boolean
	 */
	public function conectar($servidor = false, $usuario = false, $clave = false, $db = false, $puerto = false){
		if(!function_exists('mysql_connect')){
			$this->setError(__METHOD__.'1');
			$this->bConectado = false;
			return(false);
		}

		sqlazo::instCount(); //contador de instancias
		if($servidor) $this->sDDBBServidor = $servidor;
		if($usuario) $this->sDDBBUsuario = $usuario;
		if($clave) $this->sDDBBPassword = $clave;
		$this->setDDBBPuerto($puerto);

		//medicion de tiempo
		$iTiempoIni = $this->microtimeSeg();

		$this->dConexion = @ mysql_connect($this->sDDBBServidor.(!empty($this->sDDBBPuerto)?':'.$this->sDDBBPuerto:''), $this->sDDBBUsuario, $this->sDDBBPassword);

		$iTiempoFin = $this->microtimeSeg();
		$this->aTiempos['Conectar'] = number_format($iTiempoFin - $iTiempoIni, 6);
		$this->aTiempos['total'] = $iTiempoFin - $iTiempoIni; //se inicia el recuento del total de tiempo

		if($this->dConexion === false){
			$this->setError(__METHOD__.'2', $this->sDDBB, mysql_errno($this->dConexion), mysql_error($this->dConexion));
			$this->bConectado = false;
			return(false);
		}

		$this->bConectado = true;
		if($db === false) return($this->bConectado);
		else return($this->seleccionarDDBB($db));
	}

	/**
	 * Consulta a la base de datos, (si "SELECT", necesita de "sqlazo::leeFila()" para empezar a devolver resultados).
	 *
	 * @access public
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
			if(!$this->conectar($this->sDDBBServidor, $this->sDDBBUsuario, $this->sDDBBPassword, $this->sDDBB)) return(false);
		}

		if($query) $this->query($query, $pars, $cantidad, $inicial);

		//limpia los arrays de resultados, para evitar "datos fantasma"
		$this->aFila = array();
		$this->aFilaTodos = array();

		//medicion de tiempo
		$iTiempoIni = $this->microtimeSeg();

		if($this->bTrazabilidad) $this->transaccionInicio(); //se inicia el guardado de trazas

		$sTempUltIns = 0; //id de la ultima insercion, se guarda temporalmente para evitar que la pise el fin de la transaccion
		//if($this->sCharset) $this->set_charset($this->sCharset); //codificacion en que se lanzan las consultas //si se lanza despues de mysqli::init casca

		/*nota: mysql_unbuffered_query Envia una consulta SQL a MySQL, sin recuperar ni colocar en bufer las filas de resultado*/
		$this->rResultados = mysql_query($this->sQuery); //recurso de resultados

		if($this->sTipoQuery == 'insert') $sTempUltIns = mysql_insert_id();

		//seleccionar una base de datos despues de crearla
		//TODO, revisar como se selecciona una base de datos despues de su creacion
		if($this->sTipoQuery == 'create' && $this->sSubTipoQuery == 'database') $this->seleccionarDDBB($this->aTablas[0]); //despues de crear una base de datos la selecciona para las siguientes sentencias
		if($this->sTipoQuery == 'use') $this->seleccionarDDBB($this->aTablas[0]); //use `database`

		if($this->rResultados === false){
			$this->setError(__METHOD__.'1', $this->sQuery, mysql_errno($this->dConexion), mysql_error($this->dConexion));
			if($cantidad){
				$this->sQuery = str_replace('SQL_CALC_FOUND_ROWS ', '', $this->sQuery); //no es necesario en caso de error y no es algo puesto por el usuario
			}
			if($this->bTrazabilidad) $this->transaccionFinal(); //finaliza el guardado de trazas
			return(false);
		}

		$iTiempoFin = $this->microtimeSeg();
		$this->aTiempos[$this->sTipoQuery] = number_format($iTiempoFin - $iTiempoIni, 6);
		$this->aTiempos['total'] += ($iTiempoFin - $iTiempoIni);

		switch($this->sTipoQuery){
			case 'describe':
			case 'explain':
			case 'select':
			case 'show':
				if($cantidad){
					//medicion de tiempo
					$iTiempoIni = $this->microtimeSeg();

					//calculo de resultados para consultas con LIMIT
					/*$rResultados = mysql_query('SELECT FOUND_ROWS() AS totalRows', $this->dConexion);
					$aResultados = mysql_fetch_array($rResultados);*/
					$aResultados = $this->consultaInterna('SELECT FOUND_ROWS() AS totalRows');

					$this->iTotalFilas = (int) $aResultados[0]['totalRows'];
					$this->sQuery = str_replace('SQL_CALC_FOUND_ROWS ', '', $this->sQuery); //no es necesario despues de calcular el total de filas y no es algo puesto por el usuario

					$iTiempoFin = $this->microtimeSeg();
					$this->aTiempos['totalLimit'] = number_format($iTiempoFin - $iTiempoIni, 6);
					$this->aTiempos['total'] += ($iTiempoFin - $iTiempoIni);
				}
				else $this->numResultados(); //calculo del total de resultados

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

		if($this->bTrazabilidad) $this->transaccionFinal(); //finaliza el guardado de trazas; no puede estar antes del recuento, ya que este devolveria valores incorrectos al sustituir la traza a la que interesaba como ultima consulta
		return($iRet);
	}

	/**
	 * Metodo para consultas internas de la clase.
	 * Devuelve el array de resultados completo (como "sqlazo::leeFilasTodos()") o falso si ha habido error.
	 *
	 * @access protected
	 * @param string $sql Consulta SQL
	 * @return array
	 */
	protected function consultaInterna($sql){
		//TODO debe devolver al menos la id en caso de insert (tambien para udate/delete?)
		$rResultados = mysql_query($sql, $this->dConexion);
		if(!$rResultados){
			$this->setError(__METHOD__, mysql_errno(), mysql_error(), $sql);
			return(false);
		}
		$aResultados = array();

		if($rResultados !== true){
			while($aResultados[] = mysql_fetch_array($rResultados));
			if($aResultados[count($aResultados)-1] === false) array_pop($aResultados); //el anterior bucle (si todo es correcto) colocara un false en el ultimo elemento, sobra
		}

		return($aResultados);
	}

	/**
	 * Desconexion del servidor.
	 *
	 * @access public
	 * @return boolean
	 */
	public function desconectar(){
		$bool = @ mysql_free_result($this->rResultados); //fuerza liberacion de memoria, solo sirve para "SELECT" (devuelve booleano)
		if(sqlazo::instCount(-1) == 0){
			$sReturn = mysql_close($this->dConexion);
			$this->setError(__METHOD__.'1', $this->sDDBB, mysql_errno($this->dConexion), mysql_error($this->dConexion));
			$this->bConectado = true;
		}
		else $sReturn = true;

		$this->bSeleccion = false;
		$this->bConectado = false;
		return($sReturn);
	}

	/**
	 * Lee una fila de resultados.
	 *
	 * @access public
	 * @return boolean
	 */
	public function leeFila(){
		//no se pueden devolver filas si no las hay
		$sError = $this->getError();
		if(!empty($sError)){
			$this->setError(__METHOD__.'1');
			return(false);
		}

		//este metodo solo tiene sentido con consultas tipo "SELECT"
		if($this->sTipoQuery == 'select' || $this->sTipoQuery == 'describe' || $this->sTipoQuery == 'explain' || $this->sTipoQuery == 'show'){

			if(!$this->tipoArrayQuery($this->sQueryTipoArray)) return(false); //tipo de array que devolvera la query

			//medicion de tiempo
			$iTiempoIni = $this->microtimeSeg();

			$aTemp = $this->aFila; //conserva el ultimo array de resultados (la ultima fila), que sera pisada en el siguiente if si llega al final del recordset

			if($this->aFila = mysql_fetch_array($this->rResultados, $this->sQueryTipoArray)){
				$iTiempoFin = $this->microtimeSeg();
				$this->aTiempos['fila'][] = number_format($iTiempoFin - $iTiempoIni, 6);
				$this->aTiempos['total'] += ($iTiempoFin - $iTiempoIni);

				//elimina tags html y php de los resultados
				if(!empty($this->sTagsPermitidosSel) && $this->sTagsPermitidosSel != 'todos'){
					foreach($this->aFila as $clave => $valor){
						$this->aFila[$clave] = strip_tags($valor, $this->sTagsPermitidosSel);
					}
				}

				return(true);
			}
			else{
				$this->aFila = $aTemp;
				return(false);
			}
		}
		else{
			$this->setError(__METHOD__.'2');
			return(false);
		}
	}

	/**
	 * Crea un array con todas las filas de resultados.
	 *
	 * @access public
	 * @return boolean
	 */
	public function leeFilasTodos(){
		//no se pueden devolver filas si no las hay
		$sError = $this->getError();
		if(!empty($sError)){
			$this->setError(__METHOD__.'1');
			return(false);
		}

		//este metodo solo tiene sentido con consultas tipo "SELECT"
		if($this->sTipoQuery == 'describe' || $this->sTipoQuery == 'explain' || $this->sTipoQuery == 'select' || $this->sTipoQuery == 'show'){
			//recoloca el puntero en el inicio para obtener todos los resultados
			if($this->iTotalFilas > 0) $this->recPuntero();

			if(!$this->tipoArrayQuery($this->sQueryTipoArray)) return(false); //tipo de array que devolvera la query

			//medicion de tiempo
			$iTiempoIni = $this->microtimeSeg();

			while($this->aFilaTodos[] = mysql_fetch_array($this->rResultados, $this->sQueryTipoArray));
			if($this->aFilaTodos[count($this->aFilaTodos)-1] === false) array_pop($this->aFilaTodos); //el anterior bucle (si todo es correcto) colocara un false en el ultimo elemento, sobra

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

			return(true);
		}
		else{
			$this->setError(__METHOD__.'2');
			return(false);
		}
	}

	/**
	 * Lista de los campos de la consulta
	 * Devuelve tanto por posicion en el recordset de resultados como por nombre de campo.
	 *
	 * @access private
	 * @return boolean
	 */
	private function listaCampos(){
		$this->aCampos = array(); //se vacia, por si se vuelve a llamar la funcion
		if(is_resource($this->rResultados)){
			for($i=0;$i<mysql_num_fields($this->rResultados);$i++){
				//$this->aCampos[] = array('nombre' => mysql_field_name($this->rResultados, $i), 'tipo' => mysql_field_type($this->rResultados, $i), 'long' => mysql_field_len($this->rResultados, $i), 'flags' => mysql_field_flags($this->rResultados, $i));
				$this->aCampos[$i]['nombre'] = mysql_field_name($this->rResultados, $i);
				$this->aCampos[$this->aCampos[$i]['nombre']]['pos'] = $i;
				$this->aCampos[$this->aCampos[$i]['nombre']]['tipo'] = $this->aCampos[$i]['tipo'] = mysql_field_type($this->rResultados, $i);
				$this->aCampos[$this->aCampos[$i]['nombre']]['long'] = $this->aCampos[$i]['long'] = mysql_field_len($this->rResultados, $i);
				$this->aCampos[$this->aCampos[$i]['nombre']]['flags'] = $this->aCampos[$i]['flags'] = mysql_field_flags($this->rResultados, $i);
			}
			return(true);
		}
		return(false);
	}

	/**
	 * Calcular el numero de filas devueltas o afectadas por la query.
	 *
	 * @todo Ver informacion de las dos funciones usadas, pueden dar resultados erroneos
	 * @access private
	 * @return boolean
	 */
	private function numResultados(){
		switch($this->sTipoQuery){
			case 'describe':
			case 'explain':
			case 'select':
			case 'show':
				$this->iTotalFilas = mysql_num_rows($this->rResultados);
				//var_dump($this->rResultados);
				return(true);
				break;
			case 'alter':
			case 'create':
			case 'drop':
			case 'use':
			case 'delete':
			case 'insert':
			case 'update':
				$this->iTotalFilas = mysql_affected_rows($this->dConexion);
				if($this->iTotalFilas < 1) $this->iTotalFilas = false;
				return(true);
				/*TODO If the last query was a DELETE query with no WHERE clause, all of the records will have been deleted from the table but this function will return zero with MySQL versions prior to 4.1.2.*/
				break;
			case 'commit':
			case 'lock':
			case 'rollback':
			case 'set':
			case 'start': //transaction
			case 'unlock':
				break;
			default:
				$this->setError(__METHOD__.'1', $this->sTipoQuery);
				return(false);
		}
	}

	/**
	 * Construir query.
	 * si ya se ha asignado una query mediante la propiedad "sQuery" y no se desea cambiar NO llamar a esta funcion y llamar al metodo "consulta" sin parametros
	 * ej: $obj->fQuery("SELECT * FROM tabla WHERE campo=%s AND b=%s", array('1', '2'))
	 * cuando se requiera el comodin SQL "%" escribirlo doble "%%"; ej: $obj->consulta("SELECT * FROM tabla WHERE campo='%%%s%%'", array('hola')) ->producira la salida: "SELECT * FROM tabla WHERE campo='%hola%'"
	 *
	 * %s -> tomara del array de parametros una cadena de caracteres (en la misma posicion relativa)
	 * %d -> entero con signo
	 * %u -> entero sin signo
	 * %F -> flotante (PHP>4.3)
	 *
	 * @access public
	 * @param string $query Query SQL
	 * @param array $pars Parametros de la query
	 * @param integer $cantidad Numero de registros a devolver
	 * @param integer $inicial Registro inicial a devolver
	 * @return void
	 */
	public function query($query, $pars = array(), $cantidad = 0, $inicial = 0){
		$inicial = is_numeric($inicial)?$inicial:0;
		$cantidad = is_numeric($cantidad)?$cantidad:0;

		//elimina tags html y php de los parametros de la consulta
		//si los parametros no van en el array "pars" no tiene efecto
		//TODO ver mysqli::real_escape_string
		if($this->sTagsPermitidosIns != 'todos'){
			for($i=0;$i<count($pars);$i++){
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
			return(false);
		}

		//suma a la consulta "SQL_CALC_FOUND_ROWS", permite tener el total de filas en consultas limitadas con "SELECT FOUND_ROWS()", sin volver a lanzar la consulta
		if($cantidad){
				$iSelectPos = strpos(strtoupper($this->sQuery), 'SELECT ');
				if($iSelectPos !== false && strpos($this->sQuery, 'SQL_CALC_FOUND_ROWS') === false) $this->sQuery = substr($this->sQuery, 0, $iSelectPos + 7).'SQL_CALC_FOUND_ROWS '.substr($this->sQuery, $iSelectPos + 7);
				//si "$inicial=0" devuelve "$cantidad" de registros desde el primero
				//si se quieren devolver todos los que haya desde un "$inicial=x", dar como "$cantidad" un valor mayor al total de registros que pueda devolver la consulta (un numero al azar suficientemente grande, por ejemplo 123456789123456789123456789)
				$this->sQuery .= ' LIMIT '.$inicial.','.$cantidad;
		}
	}

	/**
	 * Recolocar puntero en el array de resultados (solo para SELECT).
	 *
	 * @access public
	 * @param integer $pos Posicion puntero
	 * @return boolean
	 * @abstract
	 */
	public function recPuntero($pos = 0){
		if($this->iTotalFilas) $bReturn = mysql_data_seek($this->rResultados, $pos); //falla con E_WARNING y devuelve FALSE si el result set esta vacio
		else $bReturn = false;
		if(!$bReturn){
			$this->setError(__METHOD__.'1', mysql_errno($this->dConexion), mysql_error($this->dConexion));
		}
		return($bReturn);
	}

	/**
	 * Seleccion de base de datos.
	 *
	 * @access public
	 * @param string $db Base de datos
	 * @return boolean
	 */
	public function seleccionarDDBB($db = false){
		if(!empty($db)){
			//if($this->sDDBB == $db) return(true); //sale con cierto si ya esta seleccionada la base de datos
			$this->sDDBB = $db;
		}
		elseif($this->bSeleccion) return(true); //devuelve cierto si no se requiere seleccionar una base de datos concreta, como cuando se desee crear una

		//medicion de tiempo
		$iTiempoIni = $this->microtimeSeg();

		$this->bSeleccion = @ mysql_select_db($this->sDDBB, $this->dConexion);

		$iTiempoFin = $this->microtimeSeg();
		$this->aTiempos['SeleccionarDDBB'] = number_format($iTiempoFin - $iTiempoIni, 6);
		$this->aTiempos['total'] += ($iTiempoFin - $iTiempoIni);

		if(!$this->bSeleccion){
			$this->setError(__METHOD__.'1', $this->sDDBB, mysql_errno($this->dConexion), mysql_error($this->dConexion));
		}

		//codificacion en que se lanzaran las consultas a la base de datos
		if(sqlazo::instCount(0) == 1 && $this->sCharset != '') $this->setCodificacion($this->sCharset, $this->sCollation);

		return($this->bSeleccion);
	}

	/**
	 * Cambia la codificacion con la que se lanzan las consultas a la base de datos.
	 * Devuelve falso si no se ha podido cambiar
	 *
	 * @since 2010-09-09
	 * @access public
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
		if(sqlazo::instCount(0) == 1 && $this->sCharset != ''){
			/*$this->consultaInterna("SET NAMES '$this->sCharset' COLLATE '$this->sCollation'");
			$this->consultaInterna("SET collation_connection = '".$this->sCollation."'");*/
			if($this->consultaInterna("SET CHARACTER SET '$this->sCharset'", $this->dConexion) === false){
				$this->setError(__METHOD__.'2');
			}
		}

		return($this->bSeleccion);
	}

	/**
	 * Constructor
	 * Los parametros por defecto conectan a un servidor de MYSQL local,
	 * con el usuario administrador (sin clave) y a la base de datos "test"
	 * (que existe por defecto)
	 */
	function sqlazo_mysql($servidor = 'localhost', $usuario = 'root', $password = '', $ddbb = 'test', $puerto = 3306){
		parent::sqlazo_trazabilidad($servidor, $usuario, $password, $ddbb, $puerto);

		$this->sMotor = 'mysql';
		$this->sQueryTipoArray = 'dos';

		$this->sCharset = $this->codificacion('charset');
		$this->sCollation = $this->codificacion('collation');

		$this->setErrorMensaje(array(
		'sqlazo_mysql::aseguraQuery1' => 'Numero de parametros incorrecto',
		'sqlazo_mysql::aseguraQuery2' => 'No corresponde el tipo de parametro (%%%s (num. %s)) con el valor proporcionado (%s)',
		'sqlazo_mysql::aseguraQuery3' => 'Tipo de parametro desconocido: %%%s',
		'sqlazo_mysql::conectar1' => 'No se puede conectar a base de datos. Extension MYSQL para PHP no instalada.',
		'sqlazo_mysql::conectar2' => 'No se puede conectar a la base de datos <em>%s</em>, error: [%s] %s',
		'sqlazo_mysql::consulta1' => 'La consulta <em>%s</em> ha generado el error: [%s] %s',
		'sqlazo_mysql::consulta2' => 'Tipo de consulta no soportada: %s',
		'sqlazo_mysql::consultaInterna' => 'Error ([%s] %s) en la consulta interna: %s',
		'sqlazo_mysql::desconectar1' => 'No se puede cerrar la conexion a la base de datos <em>%s</em>, error: [%s] %s',
		'sqlazo_mysql::leeFila1' => 'Ning&uacute;n resultado a devolver.',
		'sqlazo_mysql::leeFila2' => 'Intenta obtener un array de resultados de una consulta no "SELECT"',
		'sqlazo_mysql::leeFilasTodos1' => 'Ning&uacute;n resultado a devolver.',
		'sqlazo_mysql::leeFilasTodos2' => 'Intenta obtener un array de resultados de una consulta no "SELECT"',
		'sqlazo_mysql::numResultados1' => 'No es posible contar el numero de registros, no se reconoce el tipo de query: %s',
		'sqlazo_mysql::query' => 'Numero de parametros incorrecto',
		'sqlazo_mysql::recPuntero1' => 'No se ha podido recolocar el puntero debido al error: [%s] %s',
		'sqlazo_mysql::seleccionarDDBB1' => 'No se puede seleccionar la base de datos <em>%s</em>, error: [%s] %s',
		'sqlazo_mysql::setCodificacion' => 'No se pudo cambiar el juego de caracteres usado en la conexión',
		'sqlazo_mysql::tipoArrayQuery' => 'El tipo de array (<em>%s</em>) que debe devolverse es desconocido',
		'sqlazo_mysql::transaccionFinal' => 'No se ha completado correctamente la transacción.',
		'sqlazo_mysql::transaccionInicio' => 'No se ha podido iniciar la transacción.'
		));
	}

	/**
	 * Tipo de array que devolvera la query.
	 *
	 * @access private
	 * @param string $tipo Tipo de array que devolvera la consulta
	 * @return boolean
	 */
	private function tipoArrayQuery($tipo = ''){
		if(!empty($tipo)) $this->sQueryTipoArray = $tipo;

		//tambien admite las constantes de mysql (no i)
		switch($this->sQueryTipoArray){
			case 'dos': //devuelve array asociativo y numerico
				$this->sQueryTipoArray = MYSQL_BOTH;
			case MYSQL_BOTH:
				break;
			case 'aso': //devuelve array asociativo
				$this->sQueryTipoArray = MYSQL_ASSOC;
			case MYSQL_ASSOC:
				break;
			case 'num': //devuelve array numerico
				$this->sQueryTipoArray = MYSQL_NUM;
			case MYSQL_NUM:
				break;
			default:
				$this->setError(__METHOD__, $this->sQueryTipoArray);
				return(false);
		}

		return(true);
	}

	/**
	 * Concluye la transaccion, pasos posteriores a la ejecucion de la consulta solicitada.
	 * Sobreescribir en la clase hija si interesa; por ejemplo "mysqli" tiene funciones propias para controlar transacciones.
	 * Nota: en mysql solo funcionara con tablas transaccionales (como innodb)
	 *
	 * @access protected
	 * @return boolean
	 */
	protected function transaccionFinal(){
		$bRet = true;

		$sError = $this->getError();
		if(empty($sError)){
			$bRet = mysql_query('COMMIT', $this->dConexion); //fija las querys
		}
		else $bRet = mysql_query('ROLLBACK', $this->dConexion); //deshace las querys

		if(!$bRet){
			$this->setError(__METHOD__);
			return(false);
		}

		if($this->bTrazabilidad) $this->guardaTraza(); //guardado de trazas

		return(true);
	}

	/**
	 * Inicia la transaccion con la base de datos, pasos previos a lanzar la consulta solicitada.
	 * Sobreescribir en la clase hija si interesa; por ejemplo "mysqli" tiene funciones propias para controlar transacciones.
	 * Nota: en mysql solo funcionara con tablas transaccionales (como innodb)
	 *
	 * @access protected
	 * @return boolean
	 */
	protected function transaccionInicio(){
		if(!mysql_query('START TRANSACTION', $this->dConexion)){
			$this->setError(__METHOD__);
			return(false);
		}
		return(true);
	}

}
