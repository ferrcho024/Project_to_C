import pandas as pd
import numpy as np
import math as math
from scipy.spatial import distance
from fastdtw import fastdtw

def extrac_data_SIATA(datos, estacionSIATA, year, mes, dias):
# Función para extraer los datos de la estación SIATA en el mes indicado
# Devuelve un dataframe con los datos de la estación SIATA indicada
# datos  --> Dataframe con todos los datos de las estaciones SIATA
# estacionSIATA --> Estación SIATA de referencia
# fecha  --> Fecha para la cuál se hara la extracción
    
    datos = datos.loc[:,["codigoSerial", "pm25", "fechaHora", "latitud", "longitud"]]
    datos[datos['pm25'] > 250] = np.NaN
    datos[datos['pm25'] < 0] = np.NaN
    datos['fechaHora'] = pd.to_datetime(datos['fechaHora'])
    #datos= datos.set_index('fechaHora')
    fecha1 = year+'-'+mes
    fecha2 = year+'-'+str(f"{(int(mes)+1):02}")
    datos_SIATA = datos.loc[datos.loc[:,'codigoSerial'] == estacionSIATA]
    datos_SIATA = datos_SIATA.loc[(datos_SIATA['fechaHora'] >= fecha1) & (datos_SIATA['fechaHora'] < fecha2)]
    datos_SIATA.reset_index(inplace=True, drop=True)
    
    cont = 0
    for i in datos_SIATA['fechaHora']:
        datos_SIATA.loc[cont, 'fecha'] = i.date()
        datos_SIATA.loc[cont, 'hora'] = i.time()
        datos_SIATA.loc[cont, 'time'] = str(i.hour)
        cont += 1

    f_inicio = year+'-'+mes + '-01'
    f_fin = year+'-'+mes + '-' + str(dias) + ' 23:59:59'

    ref_date_range = pd.date_range(f_inicio, f_fin, freq='1H', name='fechaHora')

    datos_SIATA.sort_values('fechaHora', inplace=True)
    datos_SIATA['fechaHora'] = pd.to_datetime(datos_SIATA['fechaHora'])
    datos_SIATA = datos_SIATA.set_index('fechaHora')
    datos_SIATA = datos_SIATA.reindex(ref_date_range, fill_value=np.nan)
    #datos_SIATA.reset_index(inplace=True, drop=True)

    return datos_SIATA

def extrac_data_CS(datos, datos_SIATA, cercania, estacionSIATA, year, mes, dias):    
# Función para extraer los datos de los sensores de acuerdo con la estación SIATA más cercana
# Devuelve una lista de los nodos CS que están cercanos al nodo SIATA de referencia  --> nodos_CS
# Devuelve un dataframe con los datos de los nodos cercanos al nodo SIATA de referencia  --> pm25_c
# datos  --> Dataframe con todos los datos de los nodos de CS
# estacionSIATA --> Estación SIATA de referencia
# fecha  --> Fecha en la cuál se hara la comparación

    #ruta = "F:/PhD/Datos SIATA/Análisis/Descriptivo/Datos/"
    #datos = pd.read_csv(ruta+"datosCoordenados_CS.csv",sep=",")
    #clusters = pd.read_csv(ruta+"clusters.csv",sep=",")



    # Exptracción de los datos necesarios de los archivos
    fecha = year+'-'+mes
    pm25 = datos.loc[:,["codigoSerial", "fecha", "hora", "pm25_df", "pm25_nova"]]
    pm25 = pm25.loc[pm25.loc[:,"fecha"].str.contains(fecha)]
    pm25.reset_index(inplace=True, drop=True)

    '''# Tomar los nodos del cluster y pasarlos a una lista.
    # Se toma el daraframe de clusters y toma la fila que coincide con el valor de la estación,
    # el resultado que entrega son el índice y los valores, por lo que se toman solo los valores con el .value
    # luego se converte a una lista y se toma la posición 0, esto es un string
    # por ultimo se agregan el split y el strip para elimiar las llaves y tomar las comas como separador de la lista.  
    nodos_CS = clusters.nodosCS.loc[clusters['codigoSIATA']==estacionSIATA].values.tolist()[0].strip('][').split(', ')
    nodos_CS = [int(x) for x in nodos_CS]
    nodos_CS.sort()'''

    # Incica cuáles son los nodos CS a una distancia igual o menos a la ingresada en la variable cercanía del nodo SIATA seleccionado
    nodos_CS = nodos_cercanos(datos, datos_SIATA, cercania)
    nodos_CS = nodos_CS[estacionSIATA]

    f_inicio = fecha + '-01'
    f_fin = fecha + '-' + str(dias) + ' 23:59:59'

    ref_date_range = pd.date_range(f_inicio, f_fin, freq='1Min', name='fechaHora')

    # Filtrado de datos solo de los nodos del cluster
    pm25_c = pd.DataFrame()
    for i in nodos_CS:
        datos_nodo = pm25.loc[pm25.loc[:,"codigoSerial"] == int(i)]
        if len(datos_nodo) > 0:
            datos_nodo["fechaHora"] = datos_nodo["fecha"] + " " + datos_nodo["hora"]
            datos_nodo['fechaHora'] = pd.to_datetime(datos_nodo['fechaHora'])
            datos_nodo.set_index('fechaHora',inplace=True)
            datos_nodo = datos_nodo.reindex(ref_date_range, fill_value=np.nan)
            datos_nodo['codigoSerial'] = int(i)
            pm25_c = pd.concat([pm25_c, datos_nodo])
        else:
            nodos_CS.remove(i)

    # Elimina de la lista los nodos del cluster que no tienen datos en la fecha indicada
    '''nod = nodos_CS.copy()
    for i in nod:
        filtro = pm25_c.loc[pm25_c.loc[:,"codigoSerial"] == int(i)]
        if len(filtro.codigoSerial) == 0:
            nodos_CS.remove(i)'''
    
    '''
    extrac = pd.DataFrame()
    for i in range(24):
        hora = f"{i:02}" + ':'
        horas = pm25_c.loc[pm25_c.loc[:,"hora"].str.contains(r'^'+hora, regex=True)]
        horas['time'] = f"{i:02}" + ':00:00'
        extrac = pd.concat([extrac,horas],ignore_index=True) 
    
    extrac.reset_index(inplace=True, drop=True)
    '''
    #pm25_c = pm25_c.set_index('fechaHora')
    #pm25_c.reset_index(inplace=True, drop=True)
    
    #del ruta, datos, clusters, pm25, nod, filtro, i
    return pm25_c[['codigoSerial','pm25_df','pm25_nova']], nodos_CS

def datos_x_hora(datos, sensor, nodos = []):
# Toma un dataset que está muestreado por minutos y lo pasa a muestras por hora, sacando el promedio de cada hora
# Devuelve el dataset muestreado por horas y con la fechaHora como index
# datos --> Dataframe con los datos
# sensor --> nombre de la columna que tiene los datos de interés
# nodos --> Lista con los números de los nodos de interés

    if len(nodos) == 0: 
        nodos = datos.codigoSerial.unique().tolist()
   
    datos_x_hora = pd.DataFrame()

    for nodo in nodos:
        df = datos.loc[datos.loc[:,'codigoSerial'] == nodo]
        if len(df) == 0:
            continue
    
        if datos.index.dtype in ['<M8[ns]','datetime64[ns]']:
            df = df.resample('H').mean()
            datos_x_hora = pd.concat([datos_x_hora, df],ignore_index=False)
            datos_x_hora['codigoSerial'] = pd.to_numeric(datos_x_hora['codigoSerial'], downcast='signed')

        else:
            datosTest = pd.DataFrame()
            for i in range(24):
                hora = f"{i:02}" + ':'
                horas = df.loc[df.loc[:,"hora"].str.contains(r'^'+hora, regex=True)]
                horas['time'] = f"{i:02}" + ':00:00'
                datosTest = pd.concat([datosTest,horas],ignore_index=True) 

            #datosTest = datosTest.dropna(subset=[sensor])
            datosTest = datosTest.groupby(['fecha','time']).mean() # Saca promedio de cada hora
            datosTest = datosTest.reset_index()

            datosTest = datosTest.rename({'time': 'hora'}, axis=1)
            datosTest['codigoSerial'] = nodo
            datosTest['fechaHora'] = datosTest['fecha'] + ' ' + datosTest['hora']
            datosTest['fechaHora'] = pd.to_datetime(datosTest['fechaHora'])

            datosTest.sort_values('fechaHora', inplace=True)

            datos_x_hora = pd.concat([datos_x_hora, datosTest],ignore_index=True)

    if datos.index.dtype != 'datetime64[ns]':
        datos_x_hora.set_index('fechaHora', inplace=True)

    return datos_x_hora

def rmse(referencia, datos, nonan=False):
# Cálculo de RMSE del nodo con base en los valores de los sensores df y nova en comparación con los valores de la estación SIATA de referencia
# datos_SIATA --> Dataframe con los datos de la estación SIATA de referencia
# datos --> Dataframe con los datos DF y nova
# nodos --> Lista con los nodos a evaluar
# dq_measure --> Dataframe en el que se alamcenan los detallados de la evaluación de la calidad del datos de cada nodo y sensor
  
    feat = list(datos.columns)
    if 'codigoSerial' in feat:
        feat.remove('codigoSerial')
    col = list(referencia.columns)[0]
    nodos = list(datos.codigoSerial.unique())  

    rmse_measure = pd.DataFrame()

    for n in nodos:
        filtro = datos[datos['codigoSerial'] == n]
        filtro['SIATA'] = referencia[col]
        if nonan:
            filtro.dropna(inplace=True)
        for f in feat:
            rmse_measure.at[int(n),f] = (((filtro[f] - filtro['SIATA'])**2).mean())**.5
    
    rmse_measure.index.name = 'codigoSerial'

    return rmse_measure

def cross_correlation(referencia, datos, nonan=False):
# Correlación cruzada de dos señales para calcular la similitud
# referencia --> Datos de la señal que se usará como referencia
# datos --> Datos de la señal que se desplazará para verificar la similitud con la señal de referencia

  
    feat = list(datos.columns)
    if 'codigoSerial' in feat:
        feat.remove('codigoSerial')
    col = list(referencia.columns)[0]
    nodos = list(datos.codigoSerial.unique())  

    cross_corr = pd.DataFrame()

    for n in nodos:
        filtro = datos[datos['codigoSerial'] == n]
        filtro['SIATA'] = referencia[col]
        if nonan:
            filtro.dropna(inplace=True)
        for f in feat:
            cross_corr.at[int(n),f] = max(np.correlate(filtro[f], filtro['SIATA'], 'full'))         
    
    cross_corr.index.name = 'codigoSerial'

    return cross_corr

def dist_euclidean(referencia, datos, nonan=False):
# Cálculo de la distancia euclideana entre cada uno de los puntos entre dos señales
# referencia --> Datos de la señal que se usará como referencia
# datos --> Datos de la señal que será comparada con la señal de referencia
  
    feat = list(datos.columns)
    if 'codigoSerial' in feat:
        feat.remove('codigoSerial')
    col = list(referencia.columns)[0]
    nodos = list(datos.codigoSerial.unique())  

    euclidean = pd.DataFrame()

    for n in nodos:
        filtro = datos[datos['codigoSerial'] == n]
        filtro['SIATA'] = referencia[col]
        if nonan:
            filtro.dropna(inplace=True)
        for f in feat:
            euclidean.at[int(n),f] = distance.euclidean(filtro[f], filtro['SIATA'])         
    
    euclidean.index.name = 'codigoSerial'

    return euclidean

def dtw(referencia, datos, nonan=False):
# Cálculo de la similitud de dos señales utilizando la técnica de Dynamic Time Warping
# referencia --> Datos de la señal que se usará como referencia
# datos --> Datos de la señal que será comparada con la señal de referencia
  
    feat = list(datos.columns)
    if 'codigoSerial' in feat:
        feat.remove('codigoSerial')
    col = list(referencia.columns)[0]
    nodos = list(datos.codigoSerial.unique())  

    fdtw = pd.DataFrame()

    for n in nodos:
        filtro = datos[datos['codigoSerial'] == n]
        filtro['SIATA'] = referencia[col]
        if nonan:
            filtro.dropna(inplace=True)
        for f in feat:
            d,_ = fastdtw(filtro[f], filtro['SIATA'], dist=distance.euclidean)
            fdtw.at[int(n),f] = d
    
    fdtw.index.name = 'codigoSerial'

    return fdtw

def haversine(lon1, lat1, lon2, lat2):
    # Calcula la distancia entre dos puntos utilizando los valores de latitud y longitud y teniendo en cuenta la curvatura de la tierra
    #lon1 = Longitud punto 1
    #lat1 = Latitud punto 1
    #lon2 = Longitud punto 2
    #lat2 = Latitud punto 2
    # Retorma la distancia en Kms
    
    # Radio de la tierra
    R = 6378  
    
    #Convertir grados decimales en radianes
    lon1, lat1, lon2, lat2 = map(math.radians, [lon1, lat1, lon2, lat2])
    
    #Formula
    dlon = lon2 - lon1 #Distancia entre longitudes
    dlat = lat2 - lat1 #Distancia entre latitudes
    a = math.sin(dlat/2)**2 + math.cos(lat1) * math.cos(lat2) * math.sin(dlon/2)**2 
    c = 2 * math.asin(math.sqrt(a))
    return c * R

def nodos_cercanos(datos_CS, datos_SIATA, cercania = 2):
    # Selecciona los nodos de CS que están cercanos al nodos SIATA de referencia de acuerdo con la cercanía indicada.
    # Retorna un diccionario con la lista de nodos CS cercanos a los nodos SIATA indicados
    # cercania = valor en Kms
    
    coor_siata = datos_SIATA.groupby('codigoSerial')[['longitud', 'latitud']].mean()
    coor_CS = datos_CS.groupby('codigoSerial')[['longitud', 'latitud']].mean()

    nodos = list(coor_siata.index)
    cercanos = {}
    for n in coor_siata.index:
        cercanos[n] = []
        for l in coor_CS.index:
            distancia = haversine(coor_siata.at[n,'longitud'], coor_siata.at[n,'latitud'], coor_CS.at[l,'longitud'],coor_CS.at[l,'latitud'])
            if distancia <= cercania:
                cercanos[n].append(l)
                print('SIATA:',n,'CS',l,'distancia',distancia)
                try:
                    nodos.remove(n)
                except:
                    None
    
    return cercanos