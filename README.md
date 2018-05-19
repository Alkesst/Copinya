
# Copinya shell

### Tasks
1. Ejecutar comandos en bakground o foreground y mostrar su estado al terminar de ejecutarse los procesos en foreground. Añadir texto de estado de los procesos en Background.
2. Crear un Group Process ID distinto para el proceso padre y para los procesos hijo. Dar soporte a comandos internos de la Shell (cd). Darle el control de la terminal al proceso hijo hasta que termine y se lo devuelve al padre (foreground). El Shell debe ignorar señales como `^C`, `^Z`, etc... Sin embargo, a los hijos les debe afectar estas señales. 
3. Esperar a los procesos que estén en background para que no se queden en estado zombie o `<defunct>` añadiendo cada comando en una lista de tareas, y al recibir la señal `SIGCHLD`, ejecutar un manejador que impida que acaben en el estado `<defunct>`.
4. Suspender procesos con `^Z`, añadirlos a la lista de tareas con el estado `STOPPED`, soporte para comandos internos `fg args` para ejecutar el proceso argumento `args` de la lista de tareas en foreground; y `bg args` para ejecutar el proceso `args` de la lista de tareas en background. 


Spoiler zone:

`Copinya` means shell in catalan.

The commit [e5469f7](https://github.com/Alkesst/Copinya/commit/4ca8d1c8745e82b5b22a6199df177100365c3434) was made with the Copinya Shell.
