<?php
# Copyright (C) 2014 José M. Carnero <jm_carnero@sargazos.net>
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
if(!defined('DUNE')) die('...');

/**
 * Modelo de portada
 *
 * @author José M. Carnero
 * @since 2014-11-17
 * @version 1
 * @license http://www.gnu.org/copyleft/gpl.html
 * @package Dune
 */
class Portada_model extends Modelazo {

	function __construct(){
		parent::__construct();
	}

	public function ejemplo1(){
		$sQuery = <<<SQL
SELECT campo, campo2
FROM tabla
WHERE condicion = 1
ORDER BY campo
SQL;
		$this->oDB->consulta($sQuery, $aParams, $iNum, $iPag);
		//var_dump($this->oDB->getConsulta(), $this->oDB->totalFilas());
	
		return $this->oDB->getFilas();
	}

	function ejemplo2(){
		$this->carga('redatazo', array('motor' => 'mysql'));
		$this->redatazo->select('ent.entId, ent.estado, ent.fechaPub');
		$this->redatazo->select('con.conId, con.titulo, con.texto, con.fecha, con.idioma');
		$this->redatazo->select('usu.usuId, usu.email, usu.nombre, usu.apellidos');
		$this->redatazo->from('entradas ent');
		$this->redatazo->join('contenidos con', 'ent.entId=con.entId');
		$this->redatazo->join('usuarios usu', 'ent.usuId=usu.usuId');
		$this->redatazo->where('(usu.activo IS NOT NULL AND usu.activo = 1)');
		$this->redatazo->where('ent.estado', '+');
		$this->redatazo->order_by('ent.fechaPub', 'desc');
		$this->redatazo->order_by('con.fecha', 'desc');

		var_dump($this->redatazo->consulta());
		var_dump($this->redatazo->getConsulta());
	}

}
