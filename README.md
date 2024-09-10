# Sistemas Operativos Tarea 1
Para ejecutar el programa se debe descargar shell.c luego en la terminar de Linux ejecutar mientras se esta en el path del archivo

gcc -o shell shell.c



# Comandos Pipes soportados:
**Pipes simples como:** ls | grep archivo ; echo "Hola mundo" | tr 'a-z' 'A-Z' ; ps aux | grep bash

**Pipes multiples como:** cat archivo.txt | grep palabra | wc -l ; ps -aux | sort -nr -k 4 | head -20

# **Comando favs:**
**favs crear:** Crea el archivo de favoritos.

**favs guardar:** Guarda los comandos que se han ejecutado exitosamente en el archivo de favoritos.

**favs mostrar:** Muestra los comandos guardados en favoritos.

**favs buscar [palabra]:** Busca comandos en favoritos que contengan la palabra especificada.

**favs [número] ejecutar:** Ejecuta el comando en la posición especificada de la lista de favoritos.

**favs eliminar [número]:** Elimina un comando de la lista de favoritos.

**favs borrar:** Elimina el contenido del archivo txt favs.

# **Comando set recordatorio:**
Usa este comando para establecer un recordatorio temporal que se activará después de un número de segundos. 

Ejemplo:
mishell$: set recordatorio 10 "Recordatorio de prueba"
