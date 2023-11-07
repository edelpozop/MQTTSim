import random
import string

def generar_linea_aleatoria():
    # Generar una cadena aleatoria de números
    linea = ''.join(random.choices(string.digits, k=128))
    return linea

def crear_archivo():
    try:
        num_lineas = int(input("Ingrese el número de líneas: "))
        nombre_archivo = input("Ingrese el nombre del archivo .txt: ")

        with open(nombre_archivo, 'w') as archivo:
            for _ in range(num_lineas):
                linea = generar_linea_aleatoria()
                archivo.write(linea + '\n')

        print(f"Se ha creado el archivo {nombre_archivo} con {num_lineas} líneas de 128 bytes cada una, llenas de números aleatorios.")

    except ValueError:
        print("Error: Debe ingresar un número válido para el número de líneas.")
    except IOError:
        print("Error: No se pudo crear el archivo.")

if __name__ == "__main__":
    crear_archivo()