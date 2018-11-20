# ContikiTrafficManagement
Contiki software for traffic sensors that should represent traffic lights and corresponding car presence sensors

# Pre
Install Contiki and eventually Cooja simulator if you do not have sky-mote sensors

# Instructions
<li>Clone the repository</li>
<h3> COOJA </h3>
<ul>
  <li><kbd>cd contiki/tools/cooja</kbd></li>
  <li> <kbd>ant run</kbd> </li>
  <li>load the simulation <kbd>TrafficManagementSimulation</kbd></li>
</ul>
<h3> MOTES </h3>
<ul>
  <li> comment <kbd>#define COOJA</kbd> in <kbd>header.h</kbd> file</li>
  <li> put your motes addresses in <kbd>header.h</kbd> file</li>
  <li>Choose which file you want to upload on the device(g1,g2,tl)</li>
  <li> <kbd>./run g1</kbd> </li>
</ul>

# Contributors
[Gioele Carignani](https://github.com/gioelec/)
