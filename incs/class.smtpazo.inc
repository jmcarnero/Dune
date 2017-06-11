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
 * Envio de correo SMTP
 *
 * Ejemplo de uso:
 * \code{.php}
$oMail = new SMTPazo();
$oMail->from('from@servidor.tld');
$oMail->to('to@servidor.tld');
$oMail->cc('cc@servidor.tld');
$oMail->bcc('bcc@servidor.tld');
$oMail->subject('Ejemplo de asunto');
$oMail->message('Mensaje de ejemplo');
$oMail->smtp('smtp.servidor.tld', 'smtp_usuario', 'smtp_clave');
$bSendMail = $oMail->send();
 * \endcode
 *
 * @author José M. Carnero
 * @version 1.1b
 * @license http://www.gnu.org/copyleft/gpl.html
 * @package SMTPazo
 * @since 2015-06-14
 * @todo envío SMTP con SSL/TLS
 */
class SMTPazo {

	private $aAttachment = array();
	private $sBcc = null;
	private $sCc = null;
	private $sFrom = 'info@sargazos.net';
	private $sHeaders = '';
	private $sMessage = '';
	private $sMessageHTML = '';
	private $sReply = 'no-reply@sargazos.net';
	private $sRetornoCarro = "\r\n";
	private $sSubject = '';
	private $sTo = '';
	private $sVersion = '1.1b';

	private $sCharset = 'utf-8';

	private $sSmtpServer = null;
	private $sSmtpUser = 'anon';
	private $sSmtpPass = '';
	private $rSmtpConnect = null; //recurso de conexion SMTP

	private $aDebug = array();

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
	 * Cada llamada pone un fichero en el email
	 *
	 * @todo opcion de incluir contenido del fichero, no la url; para los que se creen en memoria y no se lleguen a guardar
	 * @param string $sFichero Ruta y nombre del fichero
	 * @return boolean
	 */
	private function attachment($sFichero){
		if(is_readable($sFichero)){
			$finfo = finfo_open(FILEINFO_MIME_TYPE);
			$ftype = finfo_file($finfo, $sFichero);
			$file = fopen($sFichero, "r");
			$attachment = fread($file, filesize($sFichero));
			$attachment = chunk_split(base64_encode($attachment));
			fclose($file);

			$msg = 'Content-Type: \'' . $ftype . '\'; name="' . basename($sFichero) . '"' . $this->sRetornoCarro;
			$msg .= "Content-Transfer-Encoding: base64" . $this->sRetornoCarro;
			$msg .= 'Content-ID: <' . basename($sFichero) . '>' . $$this->sRetornoCarro;
			//$msg .= 'X-Attachment-Id: ebf7a33f5a2ffca7_0.1' . $rn;
			$msg .= $this->sRetornoCarro . $attachment . $this->sRetornoCarro . $$this->sRetornoCarro;

			$this->aAttachment[] = $msg;
			return true;
		}

		return false; //no existe el fichero o es inaccesible
	}

	/**
	 * Direccion oculta que recibe copia del correo, Blind Carbon Copy
	 *
	 * Mismos formatos que SMTPazo::to($sTo)
	 *
	 * @param string $sBcc Direccion o direcciones separadas por comas
	 * @return void
	 */
	public function bcc($sBcc){
		//$bcc_name = "=?UTF-8?B?".base64_encode($bcc_name)."?= <bcc_mail@email.mail>";

		$this->sBcc = $this->limpiaTexto($sBcc, false);
	}

	/**
	 * Direccion que recibe copia del correo, Carbon Copy
	 *
	 * Mismos formatos que SMTPazo::to($sTo)
	 *
	 * @param string $sCc Direccion o direcciones separadas por comas
	 * @return void
	 */
	public function cc($sCc){
		//$cc_name = "=?UTF-8?B?".base64_encode($cc_name)."?= <cc_mail@email.mail>";

		$this->sCc = $this->limpiaTexto($sCc, false);
	}

	/**
	 * Compone el mensaje, sus cabeceras, antes de enviarlo
	 *
	 * @return void
	 */
	private function componer(){
		$sLimite = md5(microtime());
		$sLimiteContenido = md5(microtime());

		$aHeaders = array();

		/*$aHeaders[] .= "Sensitivity: Personal\n";
		Possible Options:
		Sensitivity: Normal, Personal, Private and Company-Confidential
		These will be recognised and handled in Outlook, Thunderbird and others.

		$aHeaders[] = "From: Sender Name <sender@domain.com>";
		$aHeaders[] = "Reply-To: Recipient Name <receiver@domain3.com>";*/

		$sServerName = empty($_SERVER['SERVER_NAME']) ? php_uname('n') : $_SERVER['SERVER_NAME']; //SERVER_NAME no tiene valor en CLI
		$sServerAddr = empty($_SERVER['SERVER_ADDR']) ? gethostbyname(php_uname('n')) : $_SERVER['SERVER_ADDR']; //SERVER_ADDR no tiene valor en CLI

		$aHeaders[] = 'Message-ID: <' . $_SERVER['REQUEST_TIME'] . md5($_SERVER['REQUEST_TIME']) . '@' . $sServerName . '>';
		if(!empty($this->sSmtpServer)){
			$aHeaders[] = 'Subject: ' . $this->sSubject;
		}
		$aHeaders[] = 'From: ' . $this->sFrom;
		if($this->sCc){
			$aHeaders[] = 'Cc: ' . $this->sCc;
		}
		if($this->sBcc){
			$aHeaders[] = 'Bcc: ' . $this->sBcc;
		}
		$aHeaders[] = 'Date: ' . date('r', $_SERVER['REQUEST_TIME']);
		$aHeaders[] = 'Reply-To: ' . $this->sReply;
		$aHeaders[] = 'Content-Type: multipart/related;boundary=' . $sLimite;
		$aHeaders[] = 'X-Mailer: SMTPazo v' . $this->sVersion; //phpversion();
		$aHeaders[] = 'Mime-Version: 1.0';
		//$aHeaders[] = 'Content-Transfer-Encoding: 7bit';
		$aHeaders[] = 'Return-Path: ' . $this->sFrom;
		$aHeaders[] = 'X-Originating-IP: ' . $sServerAddr;
		$this->sHeaders = implode($this->sRetornoCarro, $aHeaders) . $this->sRetornoCarro . $this->sRetornoCarro;

		//cuerpo del mensaje
		$aMessage = array($this->sRetornoCarro . '--' . $sLimite . $this->sRetornoCarro);
		$aMessage[] = 'Content-Type: multipart/alternative;' . $this->sRetornoCarro;
		$aMessage[] = ' boundary="' . $sLimiteContenido . '"' . $this->sRetornoCarro;

		//mensaje texto
		$aMessage[] = $this->sRetornoCarro . '--' . $sLimiteContenido . $this->sRetornoCarro;
		$aMessage[] = 'Content-Type: text/plain; charset=' . $this->sCharset . $this->sRetornoCarro;
		$aMessage[] = 'Content-Transfer-Encoding: quoted-printable' . $this->sRetornoCarro . $this->sRetornoCarro;
		$aMessage[] = $this->sMessage . $this->sRetornoCarro;

		//mensaje HTML
		if($this->sMessageHTML){
			$aMessage[] = $this->sRetornoCarro . "--" . $sLimiteContenido . $this->sRetornoCarro;
			$aMessage[] = 'Content-Type: text/html; charset=' . $this->sCharset . $this->sRetornoCarro;
			$aMessage[] = 'Content-Transfer-Encoding: quoted-printable' . $this->sRetornoCarro . $this->sRetornoCarro;
			$aMessage[] = $this->sRetornoCarro . $this->sMessageHTML . $this->sRetornoCarro;
		}

		$aMessage[] = $this->sRetornoCarro . '--' . $sLimiteContenido . '--' . $this->sRetornoCarro;

		//attachements
		foreach($this->aAttachment as $sAttach){
			if($sAttach){
				$aMessage[] = $this->sRetornoCarro . '--' . $sLimite . $this->sRetornoCarro;
				$aMessage[] = $sAttach;
			}
		}

		$aMessage[] = $this->sRetornoCarro . '--' . $sLimite . '--' . $this->sRetornoCarro;
		$this->sMensaje = implode('', $aMessage);
	}

	/**
	 * Intenta establecer las propiedades de la la conexion
	 */
	/*private function conecta(){
		//date_default_timezone_set('Europe/Madrid');

		ini_set('SMTP', $this->sHost);
		ini_set('smtp_port', $this->sPort);

		$this->sHost       = "mail.phpdocx.com"; // sets the SMTP server
		$this->sPort       = 25;                    // set the SMTP port for the GMAIL server
		$this->Username   = "contact.adphpdocx"; // SMTP account username
		$this->Password   = "jfg02lP7";        // SMTP account password
	}*/

	/**
	 * Direccion (unica) que envia el correo, es OBLIGATORIA
	 *
	 * Mismos formatos que SMTPazo::to($to)
	 *
	 * @param string $sFrom Direccion o direcciones separadas por comas
	 * @return void
	 */
	public function from($sFrom){
		//$from_name = "=?UTF-8?B?".base64_encode($from_name)."?= <from_mail@email.mail>";

		$this->sFrom = $this->limpiaTexto($sFrom, false);
	}

	/**
	 * Devuelve informacion de debug
	 *
	 * @return array
	 */
	public function getDebug(){
		return $this->aDebug;
	}

	/**
	 * Get SMTP data
	 *
	 * @return	string
	 */
	protected function _get_smtp_data(){
		$data = '';

		while($str = fgets($this->rSmtpConnect, 512)){
			$data .= $str;

			if ($str[3] === ' '){
				break;
			}
		}

		return $data;
	}

	/**
	 * Limpia el texto recibido de caracteres problematicos en el email
	 *
	 * @param string $sTexto Texto a limpiar
	 * @param boolean $bCambiaRN Si true cambia los retornos de carro problematicos, si false los elimina
	 * @param boolean $bStripTags Si true quita los tags HTML
	 * @return string
	 */
	private function limpiaTexto($sTexto, $bCambiaRN = true, $bStripTags = true){
		if($bStripTags){
			$sTexto = strip_tags($sTexto);
		}
		$sTexto = str_replace(array("\r\n", "\n", "\r"), ($bCambiaRN ? $this->sRetornoCarro : ''), $sTexto); //convierte todos los retornos de carro
		//$sTexto = str_replace("=", "=3D", $sTexto);
		$sTexto = quoted_printable_encode($sTexto);

		/*$aPrefs = array(
			'scheme' => 'Q', //"B" stands for base64 encoding scheme and "Q" stands for quoted-printable encoding scheme
			//'input-charset' => "ISO-8859-1",
			//'output-charset' => strtoupper($sCharset),
			'line-length' => 72,
			'line-break-chars' => $this->sRetornoCarro,
		);
		$sTexto = iconv_mime_encode('', $sTexto, $aPrefs);*/

		return $sTexto;
	}

	/**
	 * Texto del mensaje de correo
	 *
	 * Equal sign are email special characters. =3D is the = sign
	 *
	 * Each line should be separated with a CRLF (\r\n). Lines should not be larger than 70 characters.
	 * Caution (Windows only) When PHP is talking to a SMTP server directly, if a full stop is found on the start of a line, it is removed. To counter-act this, replace these occurrences with a double dot.
	 * $text = str_replace("\n.", "\n..", $text);
	 *
	 * @todo //TODO limpiar XSS
	 * @param string $sMensaje Texto del correo
	 * @param boolean $bEsHtml Es HTML
	 * @return void
	 */
	public function message($sMensaje, $bEsHtml = false){
		$sMensaje = $this->limpiaTexto($sMensaje, true, false);

		//In case any of our lines are larger than 70 characters, we should use wordwrap()
		$sMensaje = wordwrap($sMensaje, 70, $this->sRetornoCarro);

		if($bEsHtml){
			//$sMensaje = strip_tags($sMensaje); //TODO decidir que etiquetas se permiten y limpiar XSS
			$this->sMessageHTML = $sMensaje;
		}

		if(empty($this->sMessage) || !$bEsHtml){
			$sMensaje = htmlspecialchars($sMensaje);
			$this->sMessage = $sMensaje;
		}
	}

	/**
	 * Direccion de replica, unica
	 *
	 * @param string $sReply Direccion
	 * @return void
	 */
	public function reply($sReply){
		//$reply_name = "=?UTF-8?B?" . base64_encode($reply_name) . "?= <reply_mail@email.mail>";

		$this->sReply = $this->limpiaTexto($sReply, false);
	}

	/**
	 * Envia el correo
	 *
	 * El extandar fija que los saltos de carro (tanto en mensaje con en cabeceras) sean CRLF (\r\n)
	 *
	 * @return boolean
	 */
	public function send(){
		$this->componer();

		if(empty($this->sSmtpServer)){
			$bMail = mail($this->sTo, $this->sSubject, $this->sMensaje, $this->sHeaders);
		}
		else{ //envio con servidor SMTP
			$bMail = $this->sendSMTP();
		}

		if($bMail === false){
			//limpia posibles errores anteriores
			set_error_handler('var_dump', 0);
			@$undef_var;
			restore_error_handler();

			$aError = error_get_last();
			trigger_error('Error en el envio: [' . implode("\n", $aError) . ']', E_USER_ERROR);
		}

		return $bMail;
	}

	/**
	 * Envio de mail a servidor SMTP externo
	 *
	 * @todo controlar situaciones de error
	 * @return boolean
	 */
	private function sendSMTP(){
		if(!$this->_smtp_connect() OR !$this->_smtp_authenticate()){
			return false;
		}

		if(!$this->_send_command('from', $this->sFrom)){
			return false;
		}

		if(!$this->_send_command('to', $this->sTo)){
			return false;
		}

		if($this->sCc && !$this->_send_command('to', $this->sCc)){
			return false;
		}

		if($this->sBcc && !$this->_send_command('to', $this->sBcc)){
			return false;
		}

		if(!$this->_send_command('data')){
			return false;
		}

		// perform dot transformation on any lines that begin with a dot
		$this->_send_data($this->sHeaders . preg_replace('/^\./m', '..$1', $this->sMensaje));

		$this->_send_data('.');

		$reply = $this->_get_smtp_data();
		$this->aDebug[__METHOD__]['reply'] = $reply;

		if(strpos($reply, '250') !== 0){
			$this->aDebug[__METHOD__]['error'] = $reply;
			return false;
		}

		$this->_send_command('quit');

		return true;
	}

	/**
	 * SMTP Connect
	 *
	 * @return	string
	 */
	protected function _smtp_connect(){
		$iTimeout = 5; //tiempo de conexion SMTP en segundos
		$iSmtpPort = 25;

		if(is_resource($this->rSmtpConnect)){ //ya conectado
			return true;
		}

		$this->rSmtpConnect = fsockopen($this->sSmtpServer, $iSmtpPort, $errno, $errstr, $iTimeout);

		if(!is_resource($this->rSmtpConnect)){
			$this->aDebug[__METHOD__]['error'] = $errno . ' ' . $errstr;
			return false;
		}

		stream_set_timeout($this->rSmtpConnect, $iTimeout);
		$this->aDebug[__METHOD__]['timeout'] = $errno . ' ' . $errstr;

		if($this->_send_command('connect')){
			return $this->_send_command('hello');
		}

		$this->aDebug[__METHOD__]['error'] = 'SMTP error de conexion';
		return false; //error de conexion
	}

	// --------------------------------------------------------------------

	/**
	 * Send SMTP command
	 *
	 * @param	string
	 * @param	string
	 * @return	string
	 */
	protected function _send_command($cmd, $data = ''){
		$sServerHost = empty($_SERVER['HTTP_HOST']) ? php_uname('n') : $_SERVER['HTTP_HOST']; //HTTP_HOST no tiene valor en CLI

		switch($cmd){
			case 'connect': //solo se comprueba la cadena devuelta despues de abrir el socket
				$resp = 220;
				break;
			case 'hello':
				$this->_send_data('EHLO ' . $sServerHost);
				$resp = 250;
				break;
			case 'from':
				$this->_send_data('MAIL FROM:<' . $data . '>');
				$resp = 250;
				break;
			case 'to':
				$this->_send_data('RCPT TO:<' . $data . '>');
				$resp = 250;
				break;
			case 'data':
				$this->_send_data('DATA');
				$resp = 354;
				break;
			case 'reset':
				$this->_send_data('RSET');
				$resp = 250;
				break;
			case 'quit':
				$this->_send_data('QUIT');
				$resp = 221;
				break;
		}

		$reply = $this->_get_smtp_data();

		if((int) substr($reply, 0, 3) !== $resp){
			$this->aDebug[__METHOD__]['error'] = 'SMTP error: ' . $reply;
			return false;
		}

		if($cmd === 'quit'){
			fclose($this->rSmtpConnect);
		}

		return true;
	}


	/**
	 * SMTP Authenticate
	 *
	 * @return	bool
	 */
	protected function _smtp_authenticate(){

		if(empty($this->sSmtpUser)){ //no se intenta la autentificacion si no hay usuario
			return true;
		}
		$this->_send_data('AUTH LOGIN');
		$reply = $this->_get_smtp_data();

		if(strpos($reply, '503') === 0){ //Already authenticated
			$this->aDebug[__METHOD__]['error'] = 'ya autentificado';
			return true;
		}
		elseif(strpos($reply, '334') !== 0){
			$this->aDebug[__METHOD__]['error'] = 'error de autentificacion';
			return false;
		}

		$this->_send_data(base64_encode($this->sSmtpUser));
		$reply = $this->_get_smtp_data();

		if(strpos($reply, '334') !== 0){
			$this->aDebug[__METHOD__]['error'] = 'error de autentificacion, usuario desconocido';
			return false;
		}

		$this->_send_data(base64_encode($this->sSmtpPass));
		$reply = $this->_get_smtp_data();

		if(strpos($reply, '235') !== 0){
			$this->aDebug[__METHOD__]['error'] = 'error de autentificacion, clave desconocida';
			return false;
		}

		return true;
	}

	/**
	 * Send SMTP data
	 *
	 * @param	string	$data
	 * @return	bool
	 */
	protected function _send_data($data){
		$iTimeout = 5; //tiempo de conexion SMTP en segundos

		$data .= $this->sRetornoCarro;
		for($written = $timestamp = 0, $length = strlen($data); $written < $length; $written += $result){
			if (($result = fwrite($this->rSmtpConnect, substr($data, $written))) === false){
				break;
			}
			elseif($result === 0){
				if($timestamp === 0){
					$timestamp = time();
				}
				elseif ($timestamp < (time() - $iTimeout)){
					$result = false;
					break;
				}

				usleep(250000);
				continue;
			}
			else{
				$timestamp = 0;
			}
		}

		if ($result === false){
			$this->aDebug[__METHOD__]['error'] = 'fallo en el envio de datos SMTP';
			return false;
		}

		return true;
	}

	/**
	 * Asigna el servidor SMTP y su puerto
	 *
	 * Si se asigna servidor se interpreta automaticamente este sistema como envio
	 *
	 * @since 2016-09-22
	 * @param string $sServer Dominio o IP del servidor SMTP
	 * @param string $sUser Nombre de usuario que conectara al servidor
	 * @param string $sPasswd Clave del usuario que conectara al servidor
	 */
	public function smtp($sServer, $sUser = 'anon', $sPasswd = ''){
		$this->sSmtpServer = $sServer;
		$this->sSmtpUser = $sUser;
		$this->sSmtpPass = $sPasswd;
	}

	/**
	 * Subject del mensaje de correo
	 *
	 * @param string $sSubject Subject
	 * @return void
	 */
	public function subject($sSubject){
		//$sSubject = "=?UTF-8?B?".base64_encode($sSubject)."?=";
		$this->sSubject = $this->limpiaTexto($sSubject, false, true);
	}

	/**
	 * Guarda la direccion a la que se enviara el correo
	 *
	 * Formatos admitidos (RFC 2822):
	 * user@example.com
	 * user@example.com, anotheruser@example.com
	 * User <user@example.com>
	 * User <user@example.com>, Another User <anotheruser@example.com>
	 *
	 * The Windows implementation of mail() differs in many ways from the Unix implementation.
	 * First, it doesn't use a local binary for composing messages but only operates on direct sockets which means a MTA is needed listening on a network socket (which can either on the localhost or a remote machine).
	 * Second, the custom headers like From:, Cc:, Bcc: and Date: are not interpreted by the MTA in the first place, but are parsed by PHP.
	 * As such, the to parameter should not be an address in the form of "Something <someone@example.com>". The mail command may not parse this properly while talking with the MTA.
	 *
	 * @param string $sTo Direccion o direcciones separadas por comas
	 * @return void
	 */
	public function to($sTo){
		//$to_name = "=?UTF-8?B?" . base64_encode($to_name) . "?= <to_mail@email.mail>";
		$this->sTo = $this->limpiaTexto($sTo, false, true);
	}

}
