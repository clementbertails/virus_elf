# Variables
USER = bertailscl
SRC_FOLDER = src
BIN_FOLDER = bin
DOC_FOLDER = doc
BACKUP_FOLDER = save

# Commandes
CC = gcc
FLAGS = -c -Wall
LFLAGS = -lm -D_BSD_SOURCE

# Specifique à la compilation
SOURCES = $(wildcard $(SRC_FOLDER)/*.c)
HEADERS = $(wildcard $(SRC_FOLDER)/*.h)
OBJ = $(subst $(SRC_FOLDER)/, $(BIN_FOLDER)/, $(SOURCES:.c=.o))
PROG = $(shell basename $$PWD)
TGZ_NAME =  $(USER)-$(PROG).tgz

# Regles
all: $(BIN_FOLDER) $(PROG)

$(PROG): $(OBJ)
	$(CC) $^ -o $@ $(LFLAGS)

bin/%.o: src/%.c $(HEADERS)
	$(CC) $(FLAGS) $< -o $@

clean:
	@echo "Nettoyage du dossier..."
	@rm -rf $(BIN_FOLDER) $(BACKUP_FOLDER) $(DOC_FOLDER) $(PROG) Doxyfile *.tgz *.old
	@echo "Fin"

save:
	@echo "Sauvegarde des fichiers sources..."
	@mkdir -p  $(BACKUP_FOLDER)
	@cp -u $(SOURCES) $(HEADERS) $(BACKUP_FOLDER)/
	@echo "Fin."

restore:
	mkdir -p $(BACKUP_FOLDER)
	# copie la sauvegarde dans le dossire $(SRC_FOLDER). creer un .old de $(SRC_FOLDER) au cas ou le dossier $(BACKUP_FOLDER) soit vide
	mkdir $(SRC_FOLDER).old
	cp -u $(SRC_FOLDER) $(SRC_FOLDER).old
	rm -rf $(SRC_FOLDER)/*
	cp $(BACKUP_FOLDER) $(SRC_FOLDER)
	

docs: $(DOC_FOLDER) $(SOURCES) $(HEADERS)
	@echo "Génération de la documentation..."
# Ne pas afficher les lignes gernerees par la creation du Doxyfile
	@doxygen -g > /dev/null 
	@rm -f Doxyfile.bak
#Specifier $(SRC_FOLDER)/ comme dossier d'entree
	@sed -i 's/INPUT                  =/INPUT                  = . $(SRC_FOLDER)/g' Doxyfile 
# Ne pas afficher les lignes gernerees par la creation du html
	@doxygen Doxyfile > /dev/null
	@rm -rf $(DOC_FOLDER)/html $(DOC_FOLDER)/latex	
	@mv html latex $(DOC_FOLDER)
	@echo "Fin."

run: all
	clear
	@./$(PROG)

give: all clean 
	tar -cvzf $(TGZ_NAME) *

$(BIN_FOLDER):
	mkdir -p $(BIN_FOLDER)

$(DOC_FOLDER):
	mkdir -p $(DOC_FOLDER)

.PHONY:all clean save restore docs run give