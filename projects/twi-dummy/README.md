<!-- vim: fo=a tw=80 colorcolumn=80 syntax=markdown :
-->

TWI dummy
=========

Acts like a TWI device (i.e sets ACK flags when needed), but does not put any
actual data bytes on the bus.

SCCB implementations seem to ignore the ACK flag. This is an attempt to add the
ACK flag so one can simply use TWI periphery to communicate two wire SCCB.

SCCB tests pending as all my ATTiny85 are high power. SCCB wants 1.8V, the
normal ATTiny85 need at least 2.7V.

An order for low-power ATTiny85V is running. Another option would be to
implement software-TWI on an ATTiny10...


Authors
-------

David R. Piegdon <dgit@piegdon.de>


License
-------

All files in this repository are released unter the GNU General Public License
Version 3 or later. See the file COPYING for more information.

