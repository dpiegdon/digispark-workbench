<!-- vim: fo=a tw=80 colorcolumn=80 syntax=markdown :
-->

ATTiny85-based WS2812 fader light
=================================

WS2812 LED strip displays random colors interpolated and scrolled. Fancy party-
or mood-light.

To scroll slower/faster, change `scroll_delay`.

To interpolate more/less (see less/more different colors), change
`front_interpol_max_steps` to a different value. Try using a 2^n-1 value to get
efficient division.

To use more/less leds, change `led_count`. This must be a multiple of 3, as
R/G/B of each WS2812 LED all have their own entry.


Authors
-------

David R. Piegdon <dgit@piegdon.de>


License
-------

All files in this repository are released unter the GNU General Public License
Version 3 or later. See the file COPYING for more information.

