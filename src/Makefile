# Variáveis
CXX = g++            # Definindo o compilador (g++ para C++)
CXXFLAGS = -Wall -g   # Flags do compilador (com -Wall para exibir avisos e -g para depuração)
OBJDIR = obj          # Diretório para os arquivos objeto
# Mudamos BINDIR para a raiz do projeto (sem o diretório bin/)
BINDIR = .            # Diretório para o executável (agora é a raiz do projeto)

# Arquivos fontes
SRC = AFC.cpp utils.cpp main.cpp
# Arquivos de cabeçalho (não são compilados diretamente, mas usados nas dependências)
HEADERS = afc.h utils.h

# Arquivos objeto gerados a partir dos arquivos fontes
OBJ = $(SRC:.cpp=.o)

# Nome do executável final
EXEC = main_program

# Regra padrão para compilar o projeto
all: $(EXEC)

# Criar diretórios necessários
$(OBJDIR):
	mkdir -p $(OBJDIR)

# Regra para compilar os arquivos fonte .cpp em .o
$(OBJDIR)/%.o: %.cpp $(HEADERS) | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Regra para gerar o executável final
$(EXEC): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(EXEC)

# Limpeza dos arquivos gerados
clean:
	rm -rf $(OBJDIR) $(EXEC)
