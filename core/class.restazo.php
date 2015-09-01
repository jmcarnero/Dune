<?php
# Copyright (C) 2015 José M. Carnero <jm_carnero@sargazos.net>
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

//namespace Dune;

/**
 * Clase controlador REST
 *
 * @author José M. Carnero
 * @since 2015-05-03
 * @version 1b
 * @license http://www.gnu.org/copyleft/gpl.html
 * @package Dune
 */
class Restazo extends Controlazo {

	/**
	 * La peticion esta autentificada (true) o no
	 * Restazo::autentifica puede sobreescribirse en la clase del usuario para proveer autentificacion personalizada; si requiere parametros debera llamarse explicitamente antes de llamar a Restazo::envia
	 *
	 * @see Restazo::autentifica($aParams, $iTiempo)
	 * @var boolean
	 */
	protected $bAutentificado = false;

	/**
	 * Cabeceras recibidas en la peticion
	 *
	 * @var array
	 */
	protected $aHeaders = array(
		'HTTP_ACCEPT' => null, //Contents of the Accept: header from the current request, if there is one
		'HTTP_ACCEPT_CHARSET' => null, //Contents of the Accept-Charset: header from the current request, if there is one. Example: 'iso-8859-1,*,utf-8'
		'HTTP_ACCEPT_ENCODING' => null, //Contents of the Accept-Encoding: header from the current request, if there is one. Example: 'gzip'
		'HTTP_ACCEPT_LANGUAGE' => null, //Contents of the Accept-Language: header from the current request, if there is one. Example: 'en'
		'HTTP_HOST' => null, //Contents of the Host: header from the current request, if there is one
		'HTTP_REFERER' => null, //The address of the page (if any) which referred the user agent to the current page. This is set by the user agent. Not all user agents will set this, and some provide the ability to modify HTTP_REFERER as a feature. In short, it cannot really be trusted
		'HTTP_USER_AGENT' => null, //Contents of the User-Agent: header from the current request, if there is one. This is a string denoting the user agent being which is accessing the page. A typical example is: Mozilla/4.5 [en] (X11; U; Linux 2.2.9 i586). Among other things, you can use this value with get_browser() to tailor your page's output to the capabilities of the user agent
		'CONTENT_LENGTH' => null, //solo peticiones POST
		'CONTENT_TYPE' => null, //solo peticiones POST
		'QUERY_STRING' => null, //The query string, if any, via which the page was accessed
		'REMOTE_ADDR' => null, //The IP address from which the user is viewing the current page
		'REQUEST_METHOD' => null, //Which request method was used to access the page; i.e. 'GET', 'HEAD', 'POST', 'PUT', ...
		'REQUEST_TIME' => null, //The timestamp of the start of the request. Available since PHP 5.1.0.
		'REQUEST_URI' => null //The URI which was given in order to access this page; for instance, '/index.html'
	);

	/**
	 * En este array se recoge el de GET o POST, segun el que se haya utilizado para llegar a esta pagina; si no se queda con un array vacio
	 *
	 * @var array
	 */
	protected $aRequest = array();

	private $sClass = null; //nombre de la subclase REST (la creada por el usuario)
	private $sMethod = null; //nombre del metodo REST solicitado (sin sufijo de metodo HTTP)
	private $sHTTPMethod = null; //nombre del metodo HTTP

	function __construct(){
		$this->request();
	}

	/**
	 * Comprueba la autenticidad de la peticion
	 *
	 * Sobreescribir segun interese
	 * Debe devolver [true|false]
	 *
	 * La peticion debe tener la hora sincronizada con este servidor, ya que las peticiones son validas en el intervalo marcado por $iTiempo
	 * Si la peticion es valida el mismo hash que se recibe como parametro sirve para comprobar los datos recibidos (habran sido firmados con ese hash) y para firmar los datos devueltos
	 *
	 * Por post puede haberse recibido tambien el parametro "restSignature", que es un SHA1 de hash y los parametros recibidos por post (no es necesario si no hay parametros)
	 *
	 * @param array $aParams Nombre de los parametros recibidos por post
	 * @param integer $iTiempo Tiempo de vida del intervalo para verificar el hash
	 * return array Vacio si no es valido
	 */
	protected function autentifica($aParams = array(), $iTiempo = 120){
		$this->bAutentificado = true;

		if(!$this->bAutentificado)
			$this->envia(array('mensaje' => 'No se puede autentificar la petición'), 401);

		return $this->bAutentificado;
	}

	/**
	 * Envia la respuesta
	 * Reescribe $aDatos con el codigo de respuesta si $iResp >= 300
	 * Cada llamada a este metodo (esto es: cada metodo del usuario) puede indicar si la respuesta es cacheable, permitiendo al cliente reducir las peticiones a metodos cacheables; de momento solo tiene proposito informativo para la implementacion del cliente, y depende completamente de la decision del desarrollador
	 *
	 * @param array $aDatos Datos a enviar
	 * @param integer $iResp Codigo de respuesta HTTP
	 * @param boolean $bCacheable Indica si la respuesta enviable es cacheable //TODO indicar tambien durante cuanto tiempo debe mantenerse cache?
	 */
	protected function envia($aDatos, $iCodHTTP = 200, $bCacheable = false){
		$iCodHTTP = (int) $iCodHTTP;

		$aCodigosHTTP = array(
			200 => array('ok' => 'OK'), //Standard response for successful HTTP requests. The actual response will depend on the request method used. In a GET request, the response will contain an entity corresponding to the requested resource. In a POST request the response will contain an entity describing or containing the result of the action.
			201 => array('ok' => 'Created'), //The request has been fulfilled and resulted in a new resource being created
			202 => array('ok' => 'Accepted'), //The request has been accepted for processing, but the processing has not been completed. The request might or might not eventually be acted upon, as it might be disallowed when processing actually takes place
			204 => array('ok' => 'No Content'), //The server successfully processed the request, but is not returning any content. Usually used as a response to a successful delete request

			300 => array('redirect' => 'Multiple Choices'), //Indicates multiple options for the resource that the client may follow. It, for instance, could be used to present different format options for video, list files with different extensions, or word sense disambiguation.
			301 => array('redirect' => 'Moved Permanently'), //This and all future requests should be directed to the given URI.
			307 => array('redirect' => 'Temporary Redirect'), //(since HTTP/1.1) //In this case, the request should be repeated with another URI; however, future requests should still use the original URI. In contrast to how 302 was historically implemented, the request method is not allowed to be changed when reissuing the original request. For instance, a POST request should be repeated using another POST request.[12]
			308 => array('redirect' => 'Permanent Redirect'), //(Experimental RFC; RFC 7238) //The request, and all future requests should be repeated using another URI. 307 and 308 (as proposed) parallel the behaviours of 302 and 301, but do not allow the HTTP method to change. So, for example, submitting a form to a permanently redirected resource may continue smoothly.[13]

			400 => array('error' => 'Bad Request'), //The server cannot or will not process the request due to something that is perceived to be a client error.[14]
			401 => array('error' => 'Unauthorized'), //Similar to 403 Forbidden, but specifically for use when authentication is required and has failed or has not yet been provided. The response must include a WWW-Authenticate header field containing a challenge applicable to the requested resource. See Basic access authentication and Digest access authentication.
			402 => array('error' => 'Payment Required'), //Reserved for future use. The original intention was that this code might be used as part of some form of digital cash or micropayment scheme, but that has not happened, and this code is not usually used. YouTube uses this status if a particular IP address has made excessive requests, and requires the person to enter a CAPTCHA.[citation needed]
			403 => array('error' => 'Forbidden'), //The request was a valid request, but the server is refusing to respond to it. Unlike a 401 Unauthorized response, authenticating will make no difference.
			404 => array('error' => 'Not Found'), //The requested resource could not be found but may be available again in the future. Subsequent requests by the client are permissible
			405 => array('error' => 'Method Not Allowed'), //A request was made of a resource using a request method not supported by that resource; for example, using GET on a form which requires data to be presented via POST, or using PUT on a read-only resource.
			406 => array('error' => 'Not Acceptable'), //The requested resource is only capable of generating content not acceptable according to the Accept headers sent in the request
			408 => array('error' => 'Request Timeout'), //The server timed out waiting for the request. According to HTTP specifications: "The client did not produce a request within the time that the server was prepared to wait. The client MAY repeat the request without modifications at any later time."
			409 => array('error' => 'Conflict'), //Indicates that the request could not be processed because of conflict in the request, such as an edit conflict in the case of multiple updates.
			410 => array('error' => 'Gone'), //Indicates that the resource requested is no longer available and will not be available again. This should be used when a resource has been intentionally removed and the resource should be purged. Upon receiving a 410 status code, the client should not request the resource again in the future. Clients such as search engines should remove the resource from their indices. [citation needed] Most use cases do not require clients and search engines to purge the resource, and a "404 Not Found" may be used instead.
			411 => array('error' => 'Length Required'), //The request did not specify the length of its content, which is required by the requested resource.
			412 => array('error' => 'Precondition Failed'), //The server does not meet one of the preconditions that the requester put on the request.
			413 => array('error' => 'Request Entity Too Large'), //The request is larger than the server is willing or able to process.
			414 => array('error' => 'Request-URI Too Long'), //The URI provided was too long for the server to process. Often the result of too much data being encoded as a query-string of a GET request, in which case it should be converted to a POST request.
			415 => array('error' => 'Unsupported Media Type'), //The request entity has a media type which the server or resource does not support. For example, the client uploads an image as image/svg+xml, but the server requires that images use a different format.
			416 => array('error' => 'Requested Range Not Satisfiable'), //The client has asked for a portion of the file (byte serving), but the server cannot supply that portion. For example, if the client asked for a part of the file that lies beyond the end of the file.
			417 => array('error' => 'Expectation Failed'), //The server cannot meet the requirements of the Expect request-header field
			426 => array('error' => 'Upgrade Required'), //The client should switch to a different protocol such as TLS/1.0.
			428 => array('error' => 'Precondition Required'), //The origin server requires the request to be conditional. Intended to prevent "the 'lost update' problem, where a client GETs a resource's state, modifies it, and PUTs it back to the server, when meanwhile a third party has modified the state on the server, leading to a conflict."[16]
			429 => array('error' => 'Too Many Requests'), //The user has sent too many requests in a given amount of time. Intended for use with rate limiting schemes
			431 => array('error' => 'Request Header Fields Too Large'), //The server is unwilling to process the request because either an individual header field, or all the header fields collectively, are too large

			500 => array('server' => 'Internal Server Error'), //A generic error message, given when an unexpected condition was encountered and no more specific message is suitable.
			501 => array('server' => 'Not Implemented'), //The server either does not recognize the request method, or it lacks the ability to fulfil the request. Usually this implies future availability (e.g., a new feature of a web-service API).
			503 => array('server' => 'Service Unavailable'), //The server is currently unavailable (because it is overloaded or down for maintenance). Generally, this is a temporary state
			505 => array('server' => 'HTTP Version Not Supported'), //The server does not support the HTTP protocol version used in the request
			509 => array('server' => 'Bandwidth Limit Exceeded'), //(Apache bw/limited extension)[28] //This status code is not specified in any RFCs. Its use is unknown.
			511 => array('server' => 'Network Authentication Required'), //The client needs to authenticate to gain network access. Intended for use by intercepting proxies used to control access to the network (e.g., "captive portals" used to require agreement to Terms of Service before granting full Internet access via a Wi-Fi hotspot)
		);

		if(empty($iCodHTTP) || $iCodHTTP >= 300)
			$aDatos = array_merge($aDatos, $aCodigosHTTP[$iCodHTTP]);

		//cabeceras
		header('HTTP/1.1: ' . $iCodHTTP);
		header('Status: ' . $iCodHTTP);
		header('Content-Type: application/json');

		if($bCacheable){
			$iCacheSegundos = 86400; //86400 son 24 horas
			$iTimestamp = gmdate("D, d M Y H:i:s", time() + $iCacheSegundos) . " GMT";
			header('Expires: ' . $iTimestamp);
			header('Pragma: cache');
			header('Cache-Control: max-age=' . $iCacheSegundos);
		}
		else{
			header('Expires: Thu, 01 Jan 1970 00:16:40 GMT');
			header('Cache-Control: no-store, no-cache, must-revalidate'); // HTTP/1.1
			header('Cache-Control: post-check=0, pre-check=0', false);
			header('Pragma: no-cache');
		}

		//datos de salida
		$sRespuesta = $this->formatea($aDatos);

		header('Content-Length: ' . strlen($sRespuesta));

		exit($sRespuesta);
	}

	/**
	 * Prepara el formato de salida de los datos
	 *
	 * @param array $aDatos Datos a formatear
	 * @param string $sFormato Formato de salida
	 */
	private function formatea($aDatos, $sFormato = 'json'){
		$sRet = null;
		switch($sFormato){
			case 'json':
				$sRet = json_encode($aDatos);
				break;
			default:
		}

		return $sRet;
	}

	/**
	 * Procesar la peticion
	 * Recoge GET, POST, etc. Segun el que se haya utilizado para llegar a esta pagina
	 *
	 * @todo si el servidor no soporta alguno de los metodos devuelve 403, 404 y no llega a lanzar el servidor REST, debe controlarse la situacion en "dune.php"
	 * @param string $nombreForm Nombre del formulario
	 * @return boolean
	 */
	private function request(){
		foreach($this->aHeaders as $clave => $valor){
			if(isset($_SERVER[$clave]))
				$this->aHeaders[$clave] = $_SERVER[$clave];
		}

		if(!$this->bAutentificado){ //comprobacion de autentificacion
			$bAuth = $this->autentifica();
		}

		$this->sClass = key($_GET);

		if(isset($_GET[$this->sClass]) && $_GET[$this->sClass] != ''){
			$this->sMethod = $_GET[$this->sClass];
		}

		$this->sHTTPMethod = strtolower($this->aHeaders['REQUEST_METHOD']);
		switch($this->sHTTPMethod){
			case 'get':
				$this->aRequest = &$_GET;
				break;
			case 'post':
				$this->aRequest = &$_POST;
				break;
			case 'put': //TODO sin probar; si todo va correcto se deberia cambiar el codigo de respuesta a 201
				$this->aRequest = file_get_contents('php://input');
				break;
			case 'delete': //no recogen variables? //TODO recoger $_REQUEST?
			case 'head':
			case 'options':
				break;
			default:
				throw new ErrorException('Error fatal: Metodo HTTP [' . $this->sHTTPMethod . '] desconocido');
				$this->sClass = $this->sMethod = null;
				return false;
				break;
		}

		//llama al metodo solicitado por el usuario; debe ser metodo_metodoHTTP, ej.: $this->metodo_post()
		$sCallable = $this->sMethod . '_' . $this->sHTTPMethod;
		if(!method_exists($this, $sCallable)){
			//throw new ErrorException('Error fatal: No se puede cargar el metodo [' . $sCallable . ']');
			$this->envia(array('mensaje' => 'No se puede cargar el metodo', 'metodo' => $sCallable), 404);
		}

		$this->$sCallable();
		return true;
	}

}
