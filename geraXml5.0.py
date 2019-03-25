import os
import sys
import random


# -*- coding: utf-8 -*-



def geraTexto(): #gera o texto em si no formato xml

    texto = []
    count = 1
    fileIn = open('servidores.txt', 'r')
    dadosArq = []
    for linha in fileIn:
        dadosArq.append(linha.split(" ")) #lendo arquivo e separando em forma de lista

    texto.append('<?xml version=\'1.0\'?>\n')
    texto.append('<!DOCTYPE platform SYSTEM \"http://simgrid.gforge.inria.fr/simgrid/simgrid.dtd\">\n')
    texto.append('<!-- gerado automaticamente por Python' + sys.version + '-->\n\n')
                                                                                                            #cabeçalho
    texto.append('<platform version=\"4.1\">\n')
    texto.append('\t<zone id=\"first zone\" routing=\"Full\">\n')
    texto.append('\n')

    quant_c = int(input("Digite a quantidade de clientes (mínimo 1)\n"))

    for i in range(0, quant_c):   
        texto.append('\t\t<host id=\"client' + str(count) + '\" speed=\"1Mf\"/>\n') #clientes
        count += 1

    quant_serv_contr = dadosArq[0][0]
    quant_serv_contr = int(quant_serv_contr)
    quant_serv_disp = dadosArq[quant_serv_contr+1][0]
    quant_serv_disp = int(quant_serv_disp)

    
    count=1
    for i in range(0, quant_serv_contr):   
        texto.append('\t\t<host id=\"server' + str(count) + '\" speed=\"1Mf\"/>\n') #servidores contratados
        count += 1

    for i in range(0, quant_serv_disp):   
        texto.append('\t\t<host id=\"server' + str(count) + '\" speed=\"1Mf\"/>\n') #servidores disponíveis
        count += 1

    texto.append('\t\t<host id=\"center\" speed=\"1Mf\"/>\n') #servidor central
    texto.append('\n')
    count = 1
    quant_l = quant_serv_contr + quant_serv_disp + quant_c + 1 #quant servidores + clientes + center
    banda_c = input("Digite a banda dos clientes com a sua respectiva unidade(Ex: MBps)\n")
    lat_c = input("Digite a latência dos clientes com a sua respectiva unidade(Ex: ms)\n")
    bandas = []

    for i in range(0, quant_c): #definicao de banda e latencia dos links dos clientes para o central
        texto.append('\t\t<link id=\"' + str(count) + '\" bandwidth=\"' + banda_c + '\" latency=\"' + lat_c + '\"/>\n')
        count += 1

    for i in range(0, quant_c*(quant_serv_contr+quant_serv_disp)): #definicao de banda e latencia dos links dos clientes para os servidores
        texto.append('\t\t<link id=\"' + str(count) + '\" bandwidth=\"' + banda_c + '\" latency=\"' + lat_c + '\"/>\n')
        count += 1


    for i in range(1, quant_serv_contr+1): #colocando as bandas dos serv contr em uma lista
        bandas.append(dadosArq[i][2])

    for i in range(quant_serv_contr + 2, quant_serv_disp + quant_serv_contr + 2): #colocando as bandas dos serv disp em uma lista
        bandas.append(dadosArq[i][2])


    for i in range(0, quant_serv_contr + quant_serv_disp): #definicao de banda e latencia dos links dos servidores para o central
        texto.append('\t\t<link id=\"' + str(count) + '\" bandwidth=\"' + bandas[i] + '\" latency=\"' + lat_c + '\"/>\n')
        count += 1

        
    texto.append('\t\t<link id=\"loopback\" bandwidth=\"' + '498MBps' + '\" latency=\"' + '15us' + '\" sharing_policy=\"FATPIPE\"/>\n')

    texto.append('\n')


    
    #rotas abaixo

    count = 1

    for i in range(0, quant_c): #clientes com center
        texto.append('\t\t<route src=\"client' + str(i+1) + '\" dst=\"center\">\n')
    
        texto.append('\t\t\t<link_ctn id=\"' + str(count) + '\"/>\n')
        
                        
        texto.append('\t\t</route>\n')
        texto.append('\n')
        count += 1
    
    for i in range(0, quant_serv_contr): #servidores contratados com clientes
        for j in range(0, quant_c):
            texto.append('\t\t<route src=\"server' + str(i+1) + '\" dst=\"client' + str(j+1) + '\">\n')
           
            texto.append('\t\t\t<link_ctn id=\"' + str(count) + '\"/>\n')

                        
            texto.append('\t\t</route>\n')
            texto.append('\n')
            count += 1
            

    

    for i in range(quant_serv_contr, quant_serv_disp+1): #servidores disponiveis com clientes
        for j in range(0, quant_c):
            texto.append('\t\t<route src=\"server' + str(i+1) + '\" dst=\"client' + str(j+1) + '\">\n')

            texto.append('\t\t\t<link_ctn id=\"' + str(count) + '\"/>\n')
          
                        
            texto.append('\t\t</route>\n')
            texto.append('\n')
            count += 1
            


    for i in range(0, quant_serv_contr): #servidores contratados com center
        texto.append('\t\t<route src=\"server' + str(i+1) + '\" dst=\"center\">\n')
        
        texto.append('\t\t\t<link_ctn id=\"' + str(count) + '\"/>\n')
                        
        texto.append('\t\t</route>\n')
        texto.append('\n')
        count += 1



    for i in range(quant_serv_contr, quant_serv_disp+1): #servidores disponiveis com center
        texto.append('\t\t<route src=\"server' + str(i+1) + '\" dst=\"center\">\n')
        
        texto.append('\t\t\t<link_ctn id=\"' + str(count) + '\"/>\n')
                   
                        
        texto.append('\t\t</route>\n')
        texto.append('\n')
        count += 1

    
    texto.append('\t</zone>\n')
    texto.append('</platform>\n')

    fileIn.close()

    return texto







#main
name = input("Digite o nome do arquivo XML...\n")
name = name + '.xml'
arq = open(name, 'w')

print('Gerando',name,'...\n')

texto = geraTexto()

arq.writelines(texto)

print(name,"gerado com sucesso.\nSalvo em:", os.path.abspath(name))

arq.close()

flag = False

while(flag == False):

    option = input("\nDeseja abrir o arquivo xml gerado? (S / N)\n")
    
    if(option == 's' or option == 'S' or option == 'sim' or option == 'yes' or option == 'y'):
       os.startfile(os.path.abspath(name))
       flag = True

    elif(option == 'n' or option == 'N' or option == 'nao' or option == 'not' or option == 'no'):
       flag = True

    if(flag == False):
        print("Opção inválida. Tente novamente...")
   
arq.close()
print("Programa encerrado.")
sys.exit()
