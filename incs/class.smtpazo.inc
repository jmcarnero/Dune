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
 * Envio de correo SMTP
 *
 * @author José M. Carnero
 * @version 1b
 * @license http://www.gnu.org/copyleft/gpl.html
 * @package SMTPazo
 * @since 2015-06-14
 */
class SMTPazo {

	//private $bDebug = true;

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
	private $sVersion = '1b';

	/**
	 * Constructor
	 */
	function __construct(){
	}

	/**
	 * Destructor
	 */
	function __destruct(){
		//parent::__destruct();
	}

	/**
	 * Cada llamada pone un fichero en el email
	 *
	 * @param string $fichero Ruta y nombre del fichero
	 * @return boolean
	 */
	private function attachment($fichero){
		if(is_readable($fichero)){
			$finfo = finfo_open(FILEINFO_MIME_TYPE);
			$ftype = finfo_file($finfo, $fichero);
			$file = fopen($fichero, "r");
			$attachment = fread($file, filesize($fichero));
			$attachment = chunk_split(base64_encode($attachment));
			fclose($file);

			$msg = 'Content-Type: \'' . $ftype . '\'; name="' . basename($fichero) . '"' . $this->sRetornoCarro;
			$msg .= "Content-Transfer-Encoding: base64" . $this->sRetornoCarro;
			$msg .= 'Content-ID: <' . basename($fichero) . '>' . $$this->sRetornoCarro;
			//            $msg .= 'X-Attachment-Id: ebf7a33f5a2ffca7_0.1' . $rn;
			$msg .= $this->sRetornoCarro . $attachment . $this->sRetornoCarro . $$this->sRetornoCarro;

			$this->aAttachment[] = $msg;
			return true;
		}
		else{ //no existe el fichero o es inaccesible
			return false;
		}
	}

	/**
	 * Direccion oculta que recibe copia del correo, Blind Carbon Copy
	 *
	 * Mismos formatos que SMTPazo::to($to)
	 *
	 * @param string $bcc Direccion o direcciones separadas por comas
	 */
	function bcc($bcc){
		//$bcc_name = "=?UTF-8?B?".base64_encode($bcc_name)."?= <bcc_mail@email.mail>";

		$this->sBcc = strip_tags($bcc);
	}

	/**
	 * Direccion que recibe copia del correo, Carbon Copy
	 *
	 * Mismos formatos que SMTPazo::to($to)
	 *
	 * @param string $cc Direccion o direcciones separadas por comas
	 */
	function cc($cc){
		//$cc_name = "=?UTF-8?B?".base64_encode($cc_name)."?= <cc_mail@email.mail>";

		$this->sCc = strip_tags($cc);
	}

	/**
	 * Compone el mensaje, sus cabeceras, antes de enviarlo
	 */
	private function componer(){
		$sLimite = md5(microtime());
		$sLimiteContenido = md5(microtime());

/*
$headers .= "Sensitivity: Personal\n";
Possible Options:
Sensitivity: Normal, Personal, Private and Company-Confidential
These will be recognised and handled in Outlook, Thunderbird and others.

$headers[] = "From: Sender Name <sender@domain.com>";
$headers[] = "Reply-To: Recipient Name <receiver@domain3.com>";
$headers[] = "Subject: {$subject}";
*/

		$aHeaders[] = 'Message-ID: <' . $_SERVER['REQUEST_TIME'] . md5($_SERVER['REQUEST_TIME']) . '@' . $_SERVER['SERVER_NAME'] . '>';
		$aHeaders[] = 'From: ' . $this->sFrom;
		if($this->sBcc) $aHeaders[] = 'Bcc: ' . $this->sBcc;
		if($this->sCc) $aHeaders[] = 'Cc: ' . $this->sCc;
		$aHeaders[] = 'Date: ' . date('r', $_SERVER['REQUEST_TIME']);
		$aHeaders[] = 'Reply-To: ' . $this->sReply;
		$aHeaders[] = 'Content-Type: multipart/related;boundary=' . $sLimite;
		$aHeaders[] = 'X-Mailer: SMTPazo v' . $this->sVersion; //phpversion();
		$aHeaders[] = 'Mime-Version: 1.0';
		//$aHeaders[] = 'Content-Transfer-Encoding: 7bit';
		$aHeaders[] = 'Return-Path: ' . $this->sFrom;
		$aHeaders[] = 'X-Originating-IP: ' . $_SERVER['SERVER_ADDR'];


		$this->sHeaders = implode($this->sRetornoCarro, $aHeaders) . $this->sRetornoCarro . $this->sRetornoCarro;

		//cuerpo del mensaje
		$aMessage = array($this->sRetornoCarro . '--' . $sLimite . $this->sRetornoCarro);
		$aMessage[] = 'Content-Type: multipart/alternative;' . $this->sRetornoCarro;
		$aMessage[] = ' boundary="' . $sLimiteContenido . '"' . $this->sRetornoCarro;

		//mensaje texto
		$aMessage[] = $this->sRetornoCarro . '--' . $sLimiteContenido . $this->sRetornoCarro;
		$aMessage[] = 'Content-Type: text/plain; charset=utf-8' . $this->sRetornoCarro;
		$aMessage[] = 'Content-Transfer-Encoding: quoted-printable' . $this->sRetornoCarro;
		$aMessage[] = $this->sMessage . $this->sRetornoCarro;

		//mensaje HTML
		if($this->sMessageHTML){
			$aMessage[] = $this->sRetornoCarro . "--" . $sLimiteContenido . $this->sRetornoCarro;
			$aMessage[] = 'Content-Type: text/html; charset=utf-8' . $this->sRetornoCarro;
			$aMessage[] = 'Content-Transfer-Encoding: quoted-printable' . $this->sRetornoCarro;
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
	 * @param string $from Direccion o direcciones separadas por comas
	 */
	function from($from){
		//$from_name = "=?UTF-8?B?".base64_encode($from_name)."?= <from_mail@email.mail>";

		$this->sFrom = strip_tags($from);
	}

	/**
	 * Texto del mensaje de correo
	 *
	 * equal sign are email special characters. =3D is the = sign
	 *
	 * Each line should be separated with a CRLF (\r\n). Lines should not be larger than 70 characters.
	 * Caution (Windows only) When PHP is talking to a SMTP server directly, if a full stop is found on the start of a line, it is removed. To counter-act this, replace these occurrences with a double dot.
	 * $text = str_replace("\n.", "\n..", $text);
	 *
	 * @todo //TODO limpiar XSS
	 * @param string $message Texto del correo
	 * @param boolean $isHTML Texto HTML del correo
	 */
	function message($message, $isHTML = false){
		$message = str_replace("=", "=3D", $message);
		$message = wordwrap($message, 70, $this->sRetornoCarro);

		if(!$isHTML){
			//In case any of our lines are larger than 70 characters, we should use wordwrap()
			$message = strip_tags($message); //TODO decidir que etiquetas se permiten y limpiar XSS
			$this->sMessageHTML = $message;
		}
		else{
			$this->sMessage = $message;
		}
	}

	/**
	 * Direccion de replica, unica
	 *
	 * @param string $reply Direccion
	 */
	function reply($reply){
		//$reply_name = "=?UTF-8?B?".base64_encode($reply_name)."?= <reply_mail@email.mail>";

		$this->sReply = strip_tags($reply);
	}

	/**
	 * Envia el correo
	 *
	 * El extandar fija que los saltos de carro (tanto en mensaje con en cabeceras) sean CRLF (\r\n)
	 *
	 * @return boolean
	 */
	function send(){
		$this->componer();

		$bMail = mail($this->sTo, $this->sSubject, $this->sMensaje, $this->sHeaders);

		/*if($this->bDebug){
			var_dump(array(
				'sTo' => $this->sTo,
				'sSubject' => $this->sSubject,
				'sMessage' => $this->sMensaje,
				'sHeaders' => $this->sHeaders,
				'mail' => $bMail
			));
			die('debug');
		}*/

		return $bMail;
	}

	/**
	 * Subject del mensaje de correo
	 *
	 * @param string $subject Subject
	 */
	function subject($subject){
		$subject = strip_tags($subject);
		//$subject = "=?UTF-8?B?".base64_encode($subject)."?=";

		$this->sSubject = $subject;
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
	 * @param string $to Direccion o direcciones separadas por comas
	 */
	function to($to){
		//$to_name = "=?UTF-8?B?".base64_encode($to_name)."?= <to_mail@email.mail>";
		$this->sTo = strip_tags($to);
	}

}
