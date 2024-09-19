# Trabalho 1 - Laboratório de Sistemas

## Alunos:
Bernardo Luiz Haab

João Manoel Raupp Sganzerla

### 1. No diretório Buildroot, habilite o suporte a C++
```bash
$ make menuconfig
```
- Toolchain -> [*] Enable C++ support

Salve as configurações e selecione exit.

### 2. Habilite o suporte a rede, filesystem, e RTC
```bash
$ make linux-menuconfig
```
- Device Drivers
  - Network device support
    - (*) Intel PRO/1000 Gigabit Ethernet support
  - [*] Real Time Clock

### 3. Adicione o servido HTML à distribuição linux
Com as configurações básicas definidas, podemos criar o servidor WEB. Neste passo, vamos adicionar um script que contenha o servidor e uma página HTML responsável por fornecer informações básicas sobre o funcionamento do sistema target.

Para controle dos arquivos, criamos uma pasta chamada `custom-scripts`, onde deve ser armazenado o arquivo `server.cpp` para a execução da página HTML.

Com o arquivo `server.cpp`, é preciso realizar a compilação para a arquitetura da máquina host.

#### 3.1 Compilando `server.cpp`
Para compilar, devemos utilizar o compilador cruzado. Dentro do diretório `custom-scripts`, deve ser executado o seguinte comando:

```bash
$ ../output/host/bin/i686-linux-g++ ./server.cpp -o server
```

Com o arquivo compilado, precisamos garantir que este arquivo seja copiado para a distribuição Linux.

#### 3.2 Criando `pre-build.sh`
No arquivo `pre-build.sh` dentro do diretório `custom-scripts`, devem ser adicionadas as seguintes linhas:

```bash
cp $BASE_DIR/../custom-scripts/server $BASE_DIR/target/usr/bin
chmod +x $BASE_DIR/target/usr/bin/server
```

### 4. Compile e Rode
Para compilar, deve ser utilizado o comando `make`, que cria a distribuição Linux com o arquivo de servidor customizado.

```bash
$ make
```

Para sua execução, utiliza-se o QEMU, que deve ser instalado na máquina host:

```bash
$ sudo apt-get update
$ sudo apt-get install qemu-system
```

Com QEMU instalado, o seguinte comando executa a distribuição criada:

```bash
$ sudo qemu-system-i386 --device e1000,netdev=eth0,mac=aa:bb:cc:dd:ee:ff \
  --netdev tap,id=eth0,script=custom-scripts/qemu-ifup \
  --kernel output/images/bzImage \
  --hda output/images/rootfs.ext2 --nographic \
  --append "console=ttyS0 root=/dev/sda"
```

### 5. Executando o servidor
Para executar o servidor, utilize o seguinte comando:

```bash
$ /usr/bin/server
```