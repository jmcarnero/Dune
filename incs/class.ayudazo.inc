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

/**
 * Multiples ayudantes en forma de metodos
 *
 * @author José M. Carnero
 * @version 1
 * @license http://www.gnu.org/copyleft/gpl.html
 * @package ayudazo
 */
class Ayudazo {

	/**
	 * Constructor
	 */
	public function __construct(){
	}

	/**
	 * Destructor
	 */
	public function __destruct(){
		//parent::__destruct();
	}

	/**
	 * Recorta la cadena "$cad" a "$long" caracteres, sin cortar palabras
	 * si "$punto" la recorta aun mas, hasta el primer "$punto" (un solo caracter) desde el final de la cadena (para no cortar frases
	 *
	 * @todo excluir tags HTML "strip_tags($cad, '<a>')" deja "<a>"; controlar que no quede ninguno abierto despues de cortar la cadena (o se arrastra el tag a las demas noticias mostradas)
	 * @param string $cad Cadena a cortar
	 * @param integer $long Longitud de corte
	 * @param boolean $punto Si true corta en el punto mas proximo (previo) a la longitud de corte
	 * @return string
	 */
	public function cortaCadena($cad, $long, $punto = false){
		if(strlen($cad) <= $long) return(strip_tags($cad));
		$long -= 4; //se le resta ' ...';
		$cadCort = substr(strip_tags($cad), 0, $long);
		if($punto === false) $cadCort = substr($cadCort, 0, strrpos($cadCort, ' '));
		else $cadCort = substr($cadCort, 0, strrpos($cadCort, $punto));
		return($cadCort.' ...');
	}

	/**
	 * Recupera el contenido de una URL
	 *
	 * @param string $sUrl URL de la que recuperar información; debe ser FQDN
	 * @param array $aParams Parametros para la URL, se traducira "key": "value" a "[?|&]key=value"
	 * @return array
	 */
	function getUrl($sUrl, $aParams = array(), $sMetodo = 'GET'){
		$sMetodo = strtoupper($sMetodo);

		$sRet = array(
			'response' => null, //respuesta de la pagina: imagen, texto, json o lo que sea que genere la url pedida
			'info' => array(), //informacion de la peticion (cabeceras de la respuesta); se rellenan despues de enviar la peticion
			'error' => null
		);

		@ini_set('allow_url_fopen', true);
		$bUrlFopen = ini_get('allow_url_fopen'); //estado de la directiva allow_url_fopen
		$bCurl = function_exists('curl_init'); //estado de la extension CURL

		//preparando los parametros para la url
		$sParams = '';
		$sUrlQueryMark = '';
		if(!empty($aParams) && count($aParams)){
			if(strpos($sUrl, '?') === false){
				$sUrlQueryMark = '?';
			}

			$aAux = array();
			foreach($aParams as $sClave => $sValor){
				$aAux[] = urlencode($sClave) . '=' . urlencode($sValor);
			}

			$sParams .= implode('&', $aAux);
		}

		if($bCurl){
			$oCurl = curl_init();
			$oCurlOpts = array(
				CURLOPT_HEADER => false,
				CURLOPT_RETURNTRANSFER => true, //si no hay errores curl_exec devolvera lo que obtenga de la url

				CURLOPT_SSL_VERIFYHOST => false, //TODO no se verifica ssl, cambiar
				CURLOPT_SSL_VERIFYPEER => false,

				CURLOPT_HTTPHEADER, array(
					'Host: ' . $sUrl,
					//'Content-type: application/x-www-form-urlencoded',
					'Accept: application/json'
				),
				CURLOPT_REFERER => $sUrl,
				CURLOPT_FOLLOWLOCATION => true
			);

			if($sMetodo == 'POST'){
				$oCurlOpts[CURLOPT_POST] = true;
				$oCurlOpts[CURLOPT_POSTFIELDS] = $sParams;
				/*curl_setopt($oCurl, CURLOPT_POST, true);
				curl_setopt($oCurl, CURLOPT_POSTFIELDS, $sParams);*/
				$sUrlQueryMark = '';
				$sParams = '';
			}

			$oCurlOpts[CURLOPT_URL] = $sUrl . $sUrlQueryMark . $sParams;

			curl_setopt_array($oCurl, $oCurlOpts);
			$sRet['response'] = curl_exec($oCurl);

			if(!curl_errno($oCurl)){
				$sRet['info'] = curl_getinfo($oCurl);
			}

			if($sRet['response'] === false){
				$sRet['error'] = 'Curl error: ' . curl_error($oCurl);
			}

			curl_close($oCurl);
		}
		elseif($bUrlFopen){
			$oHeaders = stream_context_create(array(
				'http' => array(
					'method'  => $sMetodo,
					'header' => "Accept: application/json\r\n",
					//'header' => 'Content-Type: image/png',
					'user_agent' => 'Dune Framework',
					'content' => $sParams,
					//'ignore_errors' => true
				)/*,
				'ssl' => array(
					'peer_name'	=> 'itaca',
					'verify_peer' => true,
					'verify_peer_name'   => false,
					'allow_self_signed'  => true
				)*/
			));

			$sRet['response'] = file_get_contents($sUrl, false, $oHeaders);
			$sRet['info'] = $http_response_header;

			if($sRet['response'] === false){
				$sRet['error'] = 'file_get_contents error';
			}
		}

		return $sRet; //si $sRet == false no se puede recuperar la imagen de la URL
	}

	/*
	 * Two simple functions for encrypting and decrypting with RIJNDAEL 256
	//example
	echo RIJNDAEL_decrypt(RIJNDAEL_encrypt('Her name was lola!'));
	*/
	/*function RIJNDAEL_encrypt($text){
		$iv_size = mcrypt_get_iv_size(MCRYPT_RIJNDAEL_256, MCRYPT_MODE_ECB);
		$iv = mcrypt_create_iv($iv_size, MCRYPT_RAND);
		$key = "This is a very secret key";
		return base64_encode(mcrypt_encrypt(MCRYPT_RIJNDAEL_256, $key, $text, MCRYPT_MODE_ECB, $iv));
	}
	function RIJNDAEL_decrypt($text){
		$iv_size = mcrypt_get_iv_size(MCRYPT_RIJNDAEL_256, MCRYPT_MODE_ECB);
		$iv = mcrypt_create_iv($iv_size, MCRYPT_RAND);
		$key = "This is a very secret key";
		//I used trim to remove trailing spaces
		return trim(mcrypt_decrypt(MCRYPT_RIJNDAEL_256, $key, base64_decode($text), MCRYPT_MODE_ECB, $iv));
	}*/

	/**
	 * Guarda en un fichero
	 *
	 * @todo controlar permisos de creacion/edicion del fichero
	 * @param string $sFichero Fichero en el que guardar los datos
	 * @param array $aDatos Datos a guardar, cada registro se guarda en una linea, si esta vacio y $bAppend == false vaciara el fichero o lo creara vacio
	 * @param boolean $bAppend Si true pone los nuevos datos despues de los existentes, si false sustituye los viejos con los nuevos
	 * @return boolean
	 */
	function saveFile($sFichero, $aDatos, $bAppend = false){
		if(empty($sFichero)){ //nada que hacer
			return null;
		}

		if(!is_array($aDatos)){
			$aDatos = array($aDatos);
		}

		$bAppend = empty($bAppend) ? 0 : FILE_APPEND;

		$bRes = file_put_contents($sFichero, implode("\n", $aDatos), $bAppend);
		if($bRes === false){
			echo 'Error en la escritura de [' . $sFichero . ']';
			return false;
		}

		echo 'Escritura de [' . $sFichero . '] completada';
		return true;
	}

}
