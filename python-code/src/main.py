


if __name__ == '__main__':
    with open("path", "r") as f:

        fichier_entier = f.read()
        files = fichier_entier.split("\n")

    for fichier in files:
        with open(fichier, 'r') as file:
            file.open()
            file.close()