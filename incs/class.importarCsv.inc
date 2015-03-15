<?php
# Copyright (C) 2008 José M. Carnero <jm_carnero@sargazos.net>
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
 * Clase para importacion de CSV ('Comma-Separated Values') a array asociativo
 *
 * @author José M. Carnero
 */
class importarCsv{
	//variables privadas

	//variables publicas

	/*
	 * array con los campos que contiene el CSV, en el mismo orden que aparezcan
	 * los campos "--" provocan que se ignore la columna que corresponda; pueden usarse posteriormente para construir la SQL de insercion
	 */
	var $aCampos;

	/*
	 * Validaciones de campos. Vacio (por defecto) no hay validaciones, o solo los campos necesarios
	 * Asociativo: 'campo' => 'tipo de dato'
	 * Tipos de datos:
	 * - numerico
	 * - fecha
	 * - email
	 * - url
	 */
	var $aCamposVal;

	var $aCamposSepMultiple; //separadores para campos que contengan varios datos; 'nombre del campo' => 'separador'; se utiliza para traducciones

	var $aCamposExtra; //array asociativo con campos extra no contenidos en el CSV; en la forma 'nombreCampo' => 'valor'
	var $iFilaTitulos; //fila con los nombres de los campos, podrian ser los mismos que titulos del CSV, en cuyo caso debera ignorarse esta fila para importar datos ($this->iFilaInicial = $this->iFilaTitulos + 1); por defecto a false (no hay fila de titulos)
	var $iFilaInicial; //fila en la que se empezara la importacion; por defecto empieza en la fila 0
	var $iFilaFinal; //fila final a importar; a false importa hasta el final del archivo
	var $sSeparadorCampos; //separador de valores en cada fila del CSV; si es un caracter escapado debe ir entre dobles comillas; por defecto ,
	var $sCercoCampos; //caracter que encierra a los valores a recuperar; por defecto "
	var $sFormatoFechas; //cadena con el formato de las fechas, ej.: 'aaaa-dd-mm'
	var $aTraducciones; //array de traducciones, donde las claves son los terminos que deban aparecer en el CSV y sus valores las traducciones; si esta vacio se ignora la traduccion; si una clave apunta a un array, la clave sera el nombre del campo al que afectan las traducciones (exclusivamente)
	var $sFichero; //nombre del fichero que contiene los datos CSV, ruta completa
	var $aResultados; //array asociativo de resultados; cada fila contendra un array asociativo con clave igual al valor de cada $this->aCampos y valor el recuperado del CSV
	var $aMaximo; //valor maximo de cada campo
	var $aMinimo; //valor minimo de cada campo

	/*
	 * Informacion variada de resultados, en forma de array asociativo
	 * ['filas'] -> total de filas importadas
	 * ['tiempo'] -> tiempo en segundos en importar el fichero
	 */
	var $aInfo;

	//control de errores
	var $sError;

	/**
	 * Filtra los resultados (WHERE...)
	 *
	 * @access private
	 * @return boolean
	 */
	private function filtrar(){
		if(empty($this->sFiltrar)) return(true); //no hay filtros
		$sTempFiltrar = trim(str_ireplace(array('where', 'and', 'or'), array('', '&&', '||'), $this->sFiltrar));

		$this->aResultados;
		return(true);
	}

	/**
	 * Constructor
	 * Los parametros por defecto conectan a un servidor de MYSQL local,
	 * con el usuario administrador (sin clave) y a la base de datos "test"
	 * (que existe por defecto)
	 *
	 */
	function importarCsv(){
		$this->aCampos = array();
		$this->aCamposVal = array();
		$this->aCamposSepMultiple = array();
		$this->aCamposExtra = array();
		$this->iFilaTitulos = false;
		$this->iFilaInicial = 0;
		$this->iFilaFinal = false;
		$this->sSeparadorCampos = ',';
		$this->sCercoCampos = '"';
		$this->sFormatoFechas = 'dd-mm-aaaa';
		$this->aTraducciones = array();
		$this->sFichero = '';
		$this->aResultados;
		$this->aMaximo = array();
		$this->aMinimo = array();
		$this->aInfo = array('filas' => 0, 'tiempo' => 0);

		$this->sError = false;
	}

	/**
	 * Leer fichero CSV.
	 *
	 * @return boolean
	 * @access public
	 */
	function leeCsv(){
		//crear expresion regular e inicializar array de campos
		/*$sRegExp = $this->regExp();
		if($sRegExp === false) return(false);*/

		//medicion de tiempo
		$iTiempoIni = $this->microtimeSeg();

		//ini_set('auto_detect_line_endings',TRUE); //evita problemas con retornos de carro no leidos (como en ficheros de Mac)
		$rCsv = is_readable($this->sFichero)?fopen($this->sFichero, 'r'):false;

		if(!$rCsv){
			$this->sError .= 'Sin acceso al fichero '.$this->sFichero.' en el metodo "leeCsv()".'."\n";
			return(false);
		}

		//rellenar el array de resultados con las filas que interesen del CSV
		$i = 0;
		//while($aFilaCampos = fscanf($rCsv, $sRegExp)){ //version con expresiones regulares
		//TODO fgetcsv antes de php5 obliga a una longitud de recuperacion de linea (en caracteres) si existen lineas mayores dara problemas; encontrar una manera de medir el ancho de la mayor linea del CSV)
		while($aFilaCampos = fgetcsv($rCsv, filesize($this->sFichero), $this->sSeparadorCampos, $this->sCercoCampos)){

			//TODO posicionar y mover con el propio apuntador del fichero
			if($this->iFilaInicial <= $i && ($i <= $this->iFilaFinal || $this->iFilaFinal === false)){
				foreach($this->aCampos as $clave => $valor){
					if(isset($aFilaCampos[$clave]) && $valor != '--'){
						$this->aResultados[$i-$this->iFilaInicial][$valor] = $aFilaCampos[$clave];
						if(is_numeric($aFilaCampos[$clave])){
							$this->aMaximo[$valor] = max((float) $aFilaCampos[$clave], (empty($this->aMaximo[$valor])?0:(float) $this->aMaximo[$valor]));
							$this->aMinimo[$valor] = min((float) $aFilaCampos[$clave], (empty($this->aMinimo[$valor])?0:(float) $this->aMinimo[$valor]));
						}
					}
				}

				//suma a los campos recuperados del CSV los que haya en el array de campos extra
				foreach($this->aCamposExtra as $clave => $valor){
					$this->aResultados[$i-$this->iFilaInicial][$clave] = $valor;
				}

				$this->aInfo['filas']++;
			}
			elseif($this->iFilaFinal < $i) break; //sale del bucle si se sale del rango de filas a importar

			$i++;
		}

		fclose($rCsv);

		$iTiempoFin = $this->microtimeSeg();
		$this->aInfo['tiempo'] = $iTiempoFin - $iTiempoIni;

		//validaciones
		$this->validarCampos();

		//traducciones
		$this->traducirCsv();

		return(true);
	}

	/**
	 * Devuelve microtime en segundos
	 *
	 * @return float
	 * @access private
	 */
	function microtimeSeg(){
		list($mseg, $seg) = explode(" ", microtime());
		return ((float)$mseg + (float)$seg);
	}

	/**
	 * Normalizar y validar fechas.
	 * Devuelve fecha corregida o false si no es valida.
	 * $this->sFormatoFechas debe contener el formato de la fecha: 'aaaa' o 'aa' como año, 'mm' como mes y 'dd' como dia, ej.: mm,dd,aaaa (cualquier separador, pero unico)
	 *
	 * Original function was written by (bmauser AT gmail) on 16-DEC-2008
	 *
	 * @access private
	 * @param $sFecha Fecha a comprobar/convertir
	 * @return string
	 */
	private function normalizarFecha($sFecha){
		if(strlen($sFecha) >= 6 && strlen($this->sFormatoFechas) == 10){
			$separator_only = str_replace(array('m','d','a'),'', $this->sFormatoFechas);
			$separator = $separator_only[0]; // separator is first character

			if($separator && strlen($separator_only) == 2){
				$regexp = str_replace('mm', '(0?[1-9]|1[0-2])', str_replace($separator, "\\" . $separator, $this->sFormatoFechas));
				$regexp = str_replace('dd', '(0?[1-9]|[1-2][0-9]|3[0-1])', $regexp);
				$regexp = str_replace('aaaa', '(19|20)?[0-9][0-9]', $regexp); //solo para siglo 20 y 21
				//$regexp = str_replace($separator, "\\" . $separator, $regexp);
				if($regexp != $sFecha && preg_match('/'.$regexp.'\z/', $sFecha)){
					$arr = explode($separator, $sFecha); //0 dia, 1 mes, 2 año
					$arrAux = array_flip(explode($separator, $this->sFormatoFechas));
					$anio = (isset($arrAux['aaaa'])?$arrAux['aaaa']:$arrAux['aa']);
					if(@checkdate($arr[$arrAux['mm']], $arr[$arrAux['dd']], $arr[$anio]))
						return($arr[$anio].'-'.$arr[$arrAux['mm']].'-'.$arr[$arrAux['dd']]);
				}
			}
		}
		return false; //fecha no valida
	}

	/**
	 * Construye la expresion regular para leer el CSV,
	 * pone en $this->aCampos valores si no estan ya asignados (desde $this->iFilaTitulos).
	 *
	 * @return boolean
	 * @access public
	 */
	/*function regExp(){
		//construir array de campos con fila de titulos del CSV si no tiene valores
		if(!count($this->aCampos)){
			if($this->iFilaTitulos !== false){
				//$this->sError .= 'No se ha designado lista de campos.'."\n";
				//return(false);

				//si se ha definido lista de titulos se empieza a leer en la siguiente fila (si la fila inicial es menor o igual que la de titulos)
				if($this->iFilaInicial <= $this->iFilaTitulos) $this->iFilaInicial = $this->iFilaTitulos + 1;

				$rCsv = fopen($this->sFichero, 'r');
				if(!$rCsv){
					$this->sError .= 'Sin acceso al fichero '.$this->sFichero.' en el metodo "regExp()".'."\n";
					return(false);
				}
				$i = 0;
				$sFilaCampos = '';
				//TODO posicionar el puntero del fichero directamente
				//alternativa de sintaxis -> "%s\t%s\t%s\n"
				while($sFilaCampos = fscanf($rCsv, "%[\r\n]")){
				    if($i++ == $this->iFilaTitulos) break; //sale si encuentra la fila de campos
				}
				fclose($rCsv);
				if($i < $this->iFilaTitulos){
					$this->sError .= 'Valor "$this->iFilaTitulos" incorrecto, es mayor que numero de filas en el CSV.'."\n";
					return(false);
				}
			}

			$this->aCampos = explode($this->sSeparadorCampos, $sFilaCampos);
		}

		//construir expresion regular
		/*$sRegExp = false;
		if(count($this->aCampos)) $sRegExp = str_repeat('%[^'.$this->sSeparadorCampos.']'.$this->sSeparadorCampos, count($this->aCampos)-1).'%[^'.$this->sSeparadorCampos."\n]";

		return($sRegExp);* /
		return(true);
	}*/

	/**
	 * Traduce terminos que encuentre entre los resultados de la importacion.
	 * Solo se usa si $this->aCamposVal no es un array vacio; este array puede tener subarrays, en cuyo caso la clave sera el campo al que afectan las traducciones que contiene
	 *
	 * @param $aTraducciones Array de traducciones
	 * @return boolean
	 * @access public
	 */
	function traducirCsv($aTraducciones = array()){
		if(count($aTraducciones)) $this->aTraducciones = $aTraducciones;

		if(!count($this->aTraducciones)) return(true); //no hay traducciones

		for($i=0;$i<count($this->aResultados);$i++){
			foreach($this->aResultados[$i] as $clave => $valor){
				if(isset($this->aCamposSepMultiple[$clave]) && strpos($valor, $this->aCamposSepMultiple[$clave]) !== false){ //traduce un campo con multiples valores
					$aTemp = explode($this->aCamposSepMultiple[$clave], $valor);
					$aTemp2 = array();
					foreach($aTemp as $valorT){
						$aTemp2[] = $this->traducirCsvAux($clave, $valorT, $i);
					}
					$this->aResultados[$i][$clave] = implode(',', $aTemp2);
				}
				else $this->aResultados[$i][$clave] = $this->traducirCsvAux($clave, $valor, $i);
			}
		}

		return(true);
	}

	/**
	 * Auxiliar de la funcion de traduccion
	 *
	 * @param $aTraducciones Array de traducciones
	 * @return string
	 * @access private
	 */
	function traducirCsvAux($clave, $valor, $i){
		//traducciones por campos; subarray con clave = nombre_campo
		if(isset($this->aTraducciones[$clave]) && is_array($this->aTraducciones[$clave])){
			if(isset($this->aTraducciones[$clave][$valor])) return($this->aTraducciones[$clave][$valor]);
		}
		elseif(isset($this->aTraducciones[$valor]) && !is_array($this->aTraducciones[$valor])) return($this->aTraducciones[$valor]);

		return($valor);
	}

	/**
	 * Valida los campos recogidos del CSV
	 * Solo se usa si $this->aCamposVal no es un array vacio
	 *
	 * @access private
	 * @return boolean
	 */
	function validarCampos(){
		if(!count($this->aCamposVal)) return(false); //no hay validaciones

		for($i=0;$i<count($this->aResultados);$i++){
			foreach($this->aCamposVal as $clave => $valor){
				switch($valor){
					case 'email': //TODO
						//$this->sError .= 'Validacion: el campo -'.$clave.'- en la fila -'.($i+1).'- no es "email" y esta validado como tal.'."\n";
						break;
					case 'fecha':
						$sFechaTemp = $this->normalizarFecha($this->aResultados[$i][$clave]);

						if($sFechaTemp !== false) $this->aResultados[$i][$clave] = $sFechaTemp; //fecha valida
						elseif($this->aResultados[$i][$clave] != '') $this->sError .= 'Validación: el campo -'.$clave.'- en la fila -'.($i+1).'- no es "fecha" y esta validado como tal.'."\n";

						break;
					case 'numero':
						//se presupone configuracion regional europea ("." separador de miles y "," separador de decimales)
						//TODO mejorar forma de identificacion de numeros
						$aBuscTemp = array('.', '#', ' ', ','); //caracteres que no deben estar en un numero
						$aSustTemp = array('', '', '', '.'); //caracteres con los que sustituir (en el mismo orden que $aBuscTemp)

						if(is_numeric(str_replace($aBuscTemp, $aSustTemp, $this->aResultados[$i][$clave]))) $this->aResultados[$i][$clave] = str_replace($aBuscTemp, $aSustTemp, $this->aResultados[$i][$clave]);
						elseif($this->aResultados[$i][$clave] != '') $this->sError .= 'Validación: el campo -'.$clave.'- en la fila -'.($i+1).'- no es "numerico" y esta validado como tal.'."\n";

						break;
					case 'url':
					//TODO
						//$this->sError .= 'Validacion: el campo -'.$clave.'- en la fila -'.($i+1).'- no es "url" y esta validado como tal.'."\n";
						break;
					default:
						$this->sError .= 'Validación -'.$valor.'- para el campo -'.$clave.'- no reconocida: .'."\n";
						return(false);
				}
			}
		}

		return(true);
	}

}

/*
ejemplo de uso:

include_once("../inc/class.importarCsv.inc");

$oCsv = new importarCsv();
$oCsv->aCampos = array('campo1', 'campo2');
$oCsv->aCamposVal = array('campo1' => 'fecha');
//$oCsv->aCamposExtra = array('extra1' => 11);
$oCsv->sSeparadorCampos = '\t';
$oCsv->aTraducciones = array('hola' => 'Hello');
$oCsv->sFichero = '../test.csv';
$oCsv->leeCsv();

var_dump($oCsv->aResultados);
*/


/*
 * Clase para importacion de CSV ('Comma-Separated Values') a array base de datos; heredera de "importarCsv"
 */

class csvAddbb extends importarCsv {
	//variables privadas

	//variables publicas
	var $sMotor; //motor de base de datos (mysql, postgresql)
	var $sDDBBServidor; //servidor de base de datos; nombre cualificado o ip
	var $sDDBBUsuario; //usuario de la base de datos
	var $sDDBBPassword; //clave del usuario
	var $sDDBB; //base de datos a usar (schema)
	var $sTabla; //tabla en la que se haran las inserciones
	var $aQuery; //array de inserciones SQL
	var $iFilas; //cantidad de filas para inserciones multiples; false (por defecto) hace una sola insercion multiple con todo el array de resultados
	var $aResultadosQuery; //informacion de exito o fracaso de cada query

	//control de errores
	//var $sError; //heredado de la clase padre

	/**
	 * Constructor
	 * Los parametros por defecto conectan a un servidor de MYSQL local,
	 * con el usuario administrador (sin clave) y a la base de datos "test"
	 * (que existe por defecto)
	 *
	 */
	function csvAddbb(){
		parent::importarCsv(); //llama al constructor padre, para asegurar que se inicia la clase padre

		$this->sMotor = 'mysql';
		$this->sDDBBServidor = 'localhost';
		$this->sDDBBUsuario = 'root';
		$this->sDDBBPassword = '';
		$this->sDDBB = 'test';
		$this->sTabla = '';
		$this->aQuery = array();
		$this->iFilas = false;
		$this->aResultadosQuery = array();

		//$this->sError = false;
	}

	/**
	 * Construye las sentencias de insercion en la base de datos
	 * a partir del array de resultados de la importacion CSV
	 *
	 * @return boolean
	 * @access public
	 */
	//TODO opcion de UPDATE
	function construirQuery(){
		//llena el array de resultados
		if(!$this->leeCsv()){
			$this->sError .= 'Error en la llamada a "leeCsv()". desde la clase hija: '.get_class($this);
			return(false);
		}

		if(empty($this->iFilas)) $this->iFilas = 1; //si no se ha definido esta variable interpreta que solo se hace una insercion

		for($i=0;$i<count($this->aResultados);$i++){
			//preparar las cadenas para que no den problemas SQL
			//if(!get_magic_quotes_gpc()){ //no se envian por post o get
			//TODO pasar esto a clase de base de datos como array de parametros; en la siguiente sentencia se puede escribir una sucesion de '%s', tantos como campos recuperados y pasar todo el array de datos recuperados a la clase de base de datos para que los trate adecuadamente
				$this->aResultados[$i] = array_map('addslashes', $this->aResultados[$i]);
			//}

			//#TODO# revisar como construye el array de querys para $this->iFilas = 1
			$this->aQuery[$i%$this->iFilas][] = "\n('".implode("', '", $this->aResultados[$i])."')";
		}

		//añadir sentencias de insercion a cada grupo de inserciones
		$aCamposTemp = array();
		foreach($this->aResultados[0] as $clave => $valor) $aCamposTemp[] = $clave;

		$aTemp = array();
		for($i=0;$i<count($this->aQuery);$i++){
			$aTemp[] = 'INSERT INTO '.$this->sTabla.' (`'.implode('`,`',$aCamposTemp).'`) '."\n".'VALUES '.implode(',', $this->aQuery[$i]);
		}
		$this->aQuery = $aTemp;

		return(true);
	}

	/**
	 * Lanzar las inserciones SQL.
	 *
	 * @return boolean
	 * @access private
	 */
	function insertarQuery(){
		if(!count($this->aQuery)) $this->construirQuery(); //llenar el array con las querys

		//conexion a base de datos
		$dConexion = mysql_connect($this->sDDBBServidor, $this->sDDBBUsuario, $this->sDDBBPassword);
		if($dConexion === false){
			$this->sError .= 'No se puede conectar a la base de datos '.$this->sDDBB.', error:'.mysql_error($dConexion).")\n";
			return(false);
		}
		//seleccion de base de datos
		$bSeleccion = mysql_select_db($this->sDDBB, $dConexion);
		if(!$bSeleccion){
			$this->sError .= 'No se puede seleccionar la base de datos '.$this->sDDBB.', error:'.mysql_error($dConexion).")\n";
			return(false);
		}

		//consulta
		foreach($this->aQuery as $clave => $sQuery){
			$this->rResultado = mysql_query($sQuery); //array con los resultados
			if($this->rResultado === false){
				$this->sError .= 'La consulta ha generado el error-> '.mysql_error($dConexion).")\n";
				$this->aResultadosQuery[$clave] = 'Query -'.$clave.'- con error: '.mysql_error($dConexion);
				return(false);
			}
			else{
				$this->aResultadosQuery[$clave] = 'Query -'.$clave.'- ejecutada correctamente.';
			}
		}
		return(true);
	}

	/**
	 * Guardar las inserciones SQL en un archivo.
	 * Se guardan con el mismo nombre/ruta del archivo importado (concatenandole ".sql").
	 *
	 * @return boolean
	 * @access private
	 */
	function guardarQuery(){
		if(!count($this->aQuery)) $this->construirQuery(); //llenar el array con las querys
		$sSql = '';

		$rCsv = fopen($this->sFichero.'.sql', 'w');
		if(!$rCsv){
			$this->sError .= 'Sin acceso al fichero '.$this->sFichero.'.sql en el metodo "guardarQuery()".';
			return(false);
		}

		//consulta
		foreach($this->aQuery as $clave => $sQuery){
			$sSql .= str_replace('),(', "),\n(", $sQuery).';';
		}
		$sSql = str_replace(');INSERT INTO ', ");\n\nINSERT INTO ", $sSql);

		//escribir SQL en el archivo
		if(fwrite($rCsv, $sSql) === FALSE){
			$this->sError .= 'No se puede escribir en el archivo '.$this->sFichero.'.sql';
			return(false);
		}

		fclose($rCsv);

		return(true);
	}
}

/*
ejemplo de uso:

include_once("../inc/class.importarCsv.inc");

//lectura del CSV
$oCsvAddbb = new csvAddbb();
$oCsvAddbb->aCampos = array('campo1', 'campo2');
$oCsv->aCamposVal = array('campo1' => 'fecha');
//$oCsvAddbb->aCamposExtra = array('extra1' => 11);
$oCsvAddbb->sSeparadorCampos = '\t';
$oCsvAddbb->aTraducciones = array('hola' => 'Hello');
$oCsvAddbb->sFichero = '../test.csv';

//inserciones en base de datos
$oCsvAddbb->sDDBBServidor = 'localhost';
$oCsvAddbb->sDDBBUsuario = 'root';
$oCsvAddbb->sDDBBPassword = '';
$oCsvAddbb->sDDBB = 'test';
$oCsvAddbb->sTabla = 'tablaTest';

//$oCsvAddbb->insertarQuery();
$oCsvAddbb->guardarQuery();

//var_dump($oCsvAddbb->aQuery);
*/
