# Mini Twitter - Aplicación Conceptos Sistemas Operativos 🔗🧩

Implementación de la simulación de una Red Social a través del uso de hilos, procesos, pipes y alertas; de acuerdo con conceptos correspondientes al área de Sistemas Operativos.

Proyecto de manejo y aplicación de temáticas relacionadas a la implementación de procesos, hilos, pipes y señales en el funcionamiento de un sistema específco, el programa que en su ejecución recibe el nombre de Mini - Twitter busca establecer la relación de los conceptos vistos en la asignatura de "Sistemas Operativos" en el programa de Ingeniería de Sistemas en la Pontificia Universidad Javeriana de la ciudad de Bogotá entre los meses de Septiembre - Noviembre del año electivo 2021.

Las funcionalidades se encuentran orientadas bajo el contexto de realizar la simulación de la red social de Twitter hacía un contexto académico, cubriendo las funciones básicas de dicha red social, teniendo en cuenta la creación y participación de dos entidades fundamentales --> Gestor y cliente, de este modo, a través del sistema es posible: Realizar la conexión y desconexión de un usuario, seguir o dejar de seguir a un usuario, twittear un mensaje, recibir mensajes de acuerdo con los modos de ejecución (Acoplado o desacoplado). Igualmente, se implementa el uso de señales de tipo alerta para a través del gestor imprimir de manera periódica estadísticas generales de la red social.

## ¿Cómo correr los procesos Cliente y Gestor? 🚨
. Gestor: ./Gestor -r ArchivoRelaciones -t tiempoEstadisticas -m (A/D)Modo -p nombrePipeGestor -n numeroClientesIniciales
. Cliente: ./Cliente -i idCliente -p nombrePipeGestor

## Tecnologías empleadas ⚒
- C: Lenguaje de programación utilizado para la estructuración e implementación del proyecto.
- Replit: Herramienta de codificación y desarrollo colectivo entre los integrantes.
- GitHub: Versionamiento del código y flujo de trabajo.

## Desarrolladores 👨‍💻
- [Esteban Salazar Arbelaez](https://github.com/Estebans441)
- [Juan Francisco Ramirez Escobar](https://github.com/juanfra312003)
- [Sara Lorena Suárez Villamizar](https://github.com/sara0328)
