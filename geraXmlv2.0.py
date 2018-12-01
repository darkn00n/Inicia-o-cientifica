import os
import sys
import random

# -*- coding: utf-8 -*-


def buscaLista(lista, x):
    result = 0
    for i in range(len(lista)):
        if(lista[i] == x):
            result = 1
    return result


def geraTexto(): #texto em si no formato xml
    
    texto = []
    count = 1

    texto.append('<?xml version=\'1.0\'?>\n')
    texto.append('<!DOCTYPE platform SYSTEM \"http://simgrid.gforge.inria.fr/simgrid/simgrid.dtd\">\n')
    texto.append('<!-- gerado automaticamente por Python' + sys.version + '-->\n\n')

    texto.append('<platform version=\"4.1\">\n')
    texto.append('\t<zone id=\"first zone\" routing=\"Full\">\n')
    texto.append('\n')

    quant_h = int(input("Digite a quantidade de hosts (minimo 2)\n"))

    for i in range(0, quant_h):   
        texto.append('\t\t<host id=\"host' + str(count) + '\" speed=\"1Mf\"/>\n')
        count += 1

    texto.append('\n')
    count = 1
    quant_l = int(input("Digite a quantidade de links (minimo 1)\n"))
    val = int(input("Gerar banda e latencia aleatorias(digite 1) ou escolher manualmente(digite 2)?\n"))

    for i in range(0, quant_l):
        random.seed()
        if(val == 2):
            band = str(input("Digite a banda do link "  + str(count) +  " com a sua respectiva unidade(Ex: MBps) \n"))
            lat = str(input("Digite a latencia do link "  + str(count) +  " com a sua respectiva unidade Ex: ms) \n"))
            texto.append('\t\t<link id=\"' + str(count) + '\" bandwidth=\"' + band + '\" latency=\"' + lat + '\"/>\n')

        else:
            texto.append('\t\t<link id=\"' + str(count) + '\" bandwidth=\"' + str(random.randint(5,100)) + 'MBps' + '\" latency=\"' + str(random.randint(5,100)) + 'ms' + '\"/>\n')
        count += 1
    texto.append('\t\t<link id=\"loopback\" bandwidth=\"' + '498MBps' + '\" latency=\"' + '15us' + '\" sharing_policy=\"FATPIPE\"/>\n')

    texto.append('\n')
    option = int(input("Deseja sortear os links das rotas(digite 1) ou escolher manualmente(digite 2) ?\n"))
    x = int(input("Quantidade de links aleatoria(digite 1) ou escolher manualmente(digite 2)?\n"))
    
    for i in range(0, quant_h):
        for j in range(0, quant_h):
            texto.append('\t\t<route src=\"host' + str(i+1) + '\" dst=\"host' + str(j+1) + '\">\n')
            ultimos = []
            
            if(i == j):
                texto.append('\t\t\t<link_ctn id=\"loopback\"/>\n')
                texto.append('\t\t</route>\n')
                texto.append('\n')
                continue
                    
            if(option == 1):
                random.seed()
                if(x == 1):
                    q = random.randint(1, quant_l)
                else:
                    q = int(input("Quantos links deseja adicionar para o par host" + str(i+1) + " e host"+ str(j+1) + "? ("+ str(quant_l) +" disponiveis) \n"))
                while(q > quant_l):
                    q = int(input("Quantidade de links indisponivel, tente novamente! ("+ str(quant_l) +" disponiveis) \n"))
                    
                for j in range(0, q):

                    a = random.randint(1, quant_l)
                     
                    if(buscaLista(ultimos, a) == 1):
                        while(buscaLista(ultimos, a) == 1):
                            a = random.randint(1, quant_l)

                    ultimos.append(a)
                    texto.append('\t\t\t<link_ctn id=\"' + str(a) + '\"/>\n')
            else:
                q = int(input("Quantos links deseja adicionar para o par host" + str(i+1) + " e host"+ str(j+1) + "? ("+ str(quant_l) +" disponiveis) \n"))
                while(q > quant_l):
                    q = int(input("Quantidade de links indisponivel, tente novamente! ("+ str(quant_l) +" disponiveis) \n"))
                
                for k in range(0, q):
                    a = int(input("Digite o numero do link que deseja adicionar para o par host" + str(i+1) + " e host"+ str(j+1) + "\n"))
                    if(buscaLista(ultimos, a) == 1):
                        while(buscaLista(ultimos, a) == 1):
                            a = int(input("Link ja adicionado. Digite outro por favor ou digite -1 para cancelar a adicao\n"))
                            if(a == -1):
                                break
                            
                    ultimos.append(a)
                    if(a >= 0):
                        texto.append('\t\t\t<link_ctn id=\"' + str(a) + '\"/>\n')
                        print("Link " + str(a) + " adicionado.\n")
                        
            texto.append('\t\t</route>\n')
            texto.append('\n')

    texto.append('\t</zone>\n')
    texto.append('</platform>\n')

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
