# 🛠️ Probador de Cables  

Dispositivo para probar y validar el correcto funcionamiento de los cables que se usan en los equipos productivos.  
Este repositorio incluye tanto el firmware para el microcontrolador como el diseño del PCB.  


![Vista 3D del PCB](https://github.com/jnahuel-developer/Probador-de-cables/blob/main/Hardware/ProbadorDeCables/Images/ProbadorDeCables%20-%203D.jpg)  


---
---


## Funcionamiento  

 Este proyecto se puede anexar al [Probador de manillares](https://github.com/jnahuel-developer/Probador-de-manillares-de-crio), para hacer uso de sus multiplexores analógicos.  
 Gracias a ellos, se puede obtener una matriz de continuidad, y analizarla según el cable en cuestión.  
 Esto permite detectar todo tipo de anormalidades, como cortocircuitos, cables cortados, o mala ubicación de algunos contactos.  


## Ejemplo de cable utilizado


Diagrama eléctrico realizado en Altium

![Diagrama eléctrico realizado en Altium](https://github.com/jnahuel-developer/Probador-de-cables/blob/main/Cables/03.%20Im%C3%A1genes%20de%20muestra/Diagrama%20el%C3%A9ctrico.png)


Diagrama físico realizado en Altium

![Diagrama físico realizado en Altium](https://github.com/jnahuel-developer/Probador-de-cables/blob/main/Cables/03.%20Im%C3%A1genes%20de%20muestra/Diagrama%20f%C3%ADsico.png)


---
---


## 📁 Contenido del Repositorio  


📂 **Probador-de-cables/**  
├── 📁 **firmware/** → Código fuente para el microcontrolador  
├── 📁 **hardware/** → Diseño del PCB, esquemáticos y archivos Gerber  
├── 📜 **README.md** → Documentación principal  


---


## 🖼️ Modelos 3D diseñados en Altium Designer  


Vista frontal del PCB:  
![Modelado frontal](https://github.com/jnahuel-developer/Probador-de-cables/blob/main/Hardware/ProbadorDeCables/Images/ProbadorDeCables%20-%20Frente.jpg)  


Vista posterior del PCB:  
![Modelado posterior](https://github.com/jnahuel-developer/Probador-de-cables/blob/main/Hardware/ProbadorDeCables/Images/ProbadorDeCables%20-%20Dorso.jpg)  


---


## 🛠️ Herramientas Utilizadas  


- **Diseño PCB** → Altium Designer  
- **Modelado 3D** → Altium Designer  
- **Firmware Target** → MC9S08AC16CFD  
- **Compiler** → CodeWarrior HCS08 C Compiler  
- **Lenguaje** → C  


---


📩 **Contacto:** [jnahuel.developer@gmail.com](jnahuel.developer@gmail.com)  

📩 **Contacto:** [https://www.linkedin.com/in/jnahuel/](https://www.linkedin.com/in/jnahuel/)  
