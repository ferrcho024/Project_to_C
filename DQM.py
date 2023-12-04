# Funciones para medir la calidad de los datos.

import pandas as pd
import numpy as np
import time
from scipy import stats

from pyrsistent import dq

def sensors(data):

    if 'Fusion_P' in data.columns:
        df = 'Fusion_P'
    elif 'Pred_df' in data.columns:
        df = 'Pred_df'
    else:
        df = 'pm25_df'

    if 'Fusion_OLDF' in data.columns:
        nova = 'Fusion_OLDF'
    elif 'Pred_nova' in data.columns:
        nova = 'Pred_nova'
    else:
        nova = 'pm25_nova'
    
    return df, nova


def filtro(df, nube, periodo=['xhora','xdia'], dia=0):
    ''' Relaiza el filtro de los dataframe muntiindexados
    Consultar por un calor para cada multiindex. En parébtesis se indican los valores a indexar y en level se escriben 
    los nombres de las columnas de lo indices indicados.'''

    if periodo == 'xdia':
        print(df.xs((nube,2,1), level=('codigoSerial','mes','hora'))[['pm25_df_day','pm25_nova_day']].to_markdown())
    
    elif periodo == 'xhora':
        print(df.xs((nube,2,dia), level=('codigoSerial','mes','dia'))[['pm25_df_hour','pm25_nova_hour']].to_markdown())
    
    else:
        print('El periodo debe ser "xhora" o "xdia"')


def completitud_full(df, nodos, fecha, dias):
# Mide la completitud de los datos teniendo en cuenta que debe haber un dato por cada minuto
# df --> Dataframe con los datos DF y nova, con su respectiva fecha y hora.
# nodos --> Lista con los nodos a evaluar
# fecha --> año y mes a analizar
# dias --> Número de dias que tiene el mes

    s_df, s_nova = sensors(df)
    print('completitud_full:',s_df,s_nova)

    inicio = fecha+"-01 00:00:00"
    fin = fecha+'-'+str(dias)+" 23:59:00"
    contador=0
    df_window = df.copy()
    df_window["fechaHora"] = df_window["fecha"] + " " + df_window["hora"]

    df_comple = pd.DataFrame(columns =["codigoSerial","completeness_df","completeness_nova","completeness_group_df","completeness_group_nova"])

    
    for nube in nodos:
        print('Nube: ', nube)
        ref_date_range = pd.date_range(inicio, fin, freq='1Min')
        ref_date_range = pd.DataFrame(ref_date_range,columns=["ref_fechaHora"])

        #Check for any missing date
        missing_dates = ref_date_range.loc[~ref_date_range.ref_fechaHora.isin(df_window.fechaHora),"ref_fechaHora"]

        #Add missing date rows
        for missing in missing_dates:
            df_window=df_window.append({"codigoSerial":nube,"fechaHora":missing}, ignore_index = True)
        
        #Check for any missing date
        missing_dates = ref_date_range.loc[~ref_date_range.ref_fechaHora.isin(df_window.fechaHora),"ref_fechaHora"]
        
        #Check for missing data
        missing_data_df=np.count_nonzero(np.isnan(df_window[s_df]))
        missing_data_nova=np.count_nonzero(np.isnan(df_window[s_nova]))
        comp_df=100*(1-missing_data_df/np.size(df_window[s_df]))
        comp_nova=100*(1-missing_data_nova/np.size(df_window[s_nova]))
        
        
        if comp_df<75:
            group_df=0
        elif 75<=comp_df:
            group_df=1
        
        if comp_nova<75:
            group_nova=0
        elif  75<=comp_nova:
            group_nova=1  

        df_comple=df_comple.append({"codigoSerial":nube,"completeness_df":comp_df,"completeness_nova":comp_nova,"completeness_group_df":group_df,"completeness_group_nova":group_nova}, ignore_index = True)

    return df_comple

def dq_df(datos):
# Entrega un dataframe con los nodos como índice

    lista = datos.codigoSerial.unique().tolist()
    dq_measure = pd.DataFrame(index=lista)

    return dq_measure

def completitud(df, nodos, dq_measure, debug = 'N'):
# Mide la completitud de los datos teniendo en cuenta que debe haber un dato por cada minuto
# df --> Dataframe con los datos DF y nova, con su respectiva fecha y hora.
# nodos --> Lista con los nodos a evaluar
# dq_measure --> Dataframe en el que se alamcenan los detallados de la evaluación de la calidad del datos de cada nodo y sensor
# dias --> Número de dias que tiene el mes
# n --> Número de datos que debería tener un día.

    s_df, s_nova = sensors(df)
    print('completitud:',s_df,s_nova)

    # Verifica si hay datos en el dataframe de medida de la calidad de los datos.
    if len(dq_measure) == 0:
        dq_measure = dq_df(df)

    #df_window = df.copy()
    #df_window["fechaHora"] = df_window["fecha"] + " " + df_window["hora"]
    #df_window["fechaHora"] = pd.to_datetime(df_window["fechaHora"])

    #print(df_window)

    if debug == 'S':
        print('******* COMPLETITUD *********')

    for nube in nodos:
        datos = df.loc[df.loc[:,'codigoSerial'] == nube]

        comp_df = datos[s_df].count()/len(datos)
        comp_nova = datos[s_nova].count()/len(datos)

        dq_measure.loc[nube,"comp_"+s_df.split('_')[1]] = comp_df
        dq_measure.loc[nube,"comp_"+s_nova.split('_')[1]] = comp_nova

        if debug == 'S':
            print('Nube: ', nube, '-- Datos '+s_df.split('_')[1]+': ', datos[s_df].count(), '-- de: ', len(datos), '-- Completitud: ', comp_df)
            print('Nube: ', nube, '-- Datos '+s_nova.split('_')[1]+': ', datos[s_nova].count(), '-- de: ', len(datos), '-- Completitud: ', comp_nova)
    
    if debug == 'S':
        print('\n')
    
    try: 
        df.index.month
        detallado = pd.DataFrame()
        detallado['pm25_df_hour'] = df.groupby([df.codigoSerial, df.index.month, df.index.day, df.index.hour])[s_df].count()/60
        detallado['pm25_df_day'] = detallado.groupby(level=[0,1,2]).pm25_df_hour.mean()
        detallado['pm25_df_month'] = detallado.groupby(level=[0,1]).pm25_df_day.mean()
        detallado['pm25_nova_hour'] = df.groupby([df.codigoSerial, df.index.month, df.index.day, df.index.hour])[s_nova].count()/60
        detallado['pm25_nova_day'] = detallado.groupby(level=[0,1,2]).pm25_nova_hour.mean()
        detallado['pm25_nova_month'] = detallado.groupby(level=[0,1]).pm25_nova_day.mean()

        detallado.index.set_names(['codigoSerial', 'mes', 'dia', 'hora'], inplace=True)

        return dq_measure, detallado
    
    except :
        return dq_measure

def incertidumbre(df, nodos, dq_measure, val= 0, debug = 'N'):
# Cálculo de la incertidumbre del nodo con base en los valores de los sensores df y nova
# df --> Dataframe con los datos DF y nova
# nodos --> Lista con los nodos a evaluar
# dq_measure --> Dataframe en el que se alamcenan los detallados de la evaluación de la calidad del datos de cada nodo y sensor
# val  --> Número de valores en caso de calcular la incertidumbr por lapsos de tiempo, val representa la cantidad de datos de esa ventana.
    
    s_df, s_nova = sensors(df)
    print('incertidumbre:',s_df,s_nova)

    # Verifica si hay datos en el dataframe de medida de la calidad de los datos.
    if len(dq_measure) == 0:
        dq_measure = dq_df(df)
    
    if debug == 'S':
        print('******* INCERTIDUMBRE *********')
    
    for nube in nodos:
        datos = df.loc[df.loc[:,'codigoSerial'] == nube]
        

        #incer = np.sqrt((datos.pm25_df-datos.pm25_nova).pow(2).mean()/2)/((datos.pm25_df+datos.pm25_nova).mean()/2)
        incer = np.sqrt(((datos[s_df] - datos[s_nova]).pow(2).sum())/(2*(datos[s_df] + datos[s_nova]).count()*((datos[s_df] + datos[s_nova]).mean())**2))

        incer = max(0,1-incer)

        dq_measure.loc[nube,"uncer"] = incer
        
        if debug == 'S':
            print('Nube: ', nube, '-- Incertidumbre: ', incer)

    if debug == 'S':
        print('\n')
    del nube, datos, incer
    return dq_measure


def incertidumbre2(df, ref, nodos, dq_measure, val= 0, debug = 'N'):
# Cálculo de la incertidumbre del nodo con base en los valores de los sensores df y nova
# df --> Dataframe con los datos DF y nova
# ref --> Dataframe con los datos del nodo de referencia, en este caso, los datos de la estación SIATA de referencia
# nodos --> Lista con los nodos a evaluar
# dq_measure --> Dataframe en el que se alamcenan los detallados de la evaluación de la calidad del datos de cada nodo y sensor
# val  --> Número de valores en caso de calcular la incertidumbr por lapsos de tiempo, val representa la cantidad de datos de esa ventana.
    
    s_df, s_nova = sensors(df)
    print('incertidumbre2:',s_df,s_nova)

    # Verifica si hay datos en el dataframe de medida de la calidad de los datos.
    if len(dq_measure) == 0:
        dq_measure = dq_df(df)
    
    if debug == 'S':
        print('******* INCERTIDUMBRE *********')

    #cs_df = datos.groupby([datos.codigoSerial, datos.index.month, datos.index.day, datos.index.hour])[s_df].first()
    cs_df = df.groupby([df.codigoSerial, df.index.month, df.index.day, df.index.hour])[s_df].mean()
    #cs_df = datos.groupby(['codigoSerial']).resample('H').mean()[s_df].values
    cs_df.index.set_names(['codigoSerial', 'mes', 'dia', 'hora'], inplace=True)
    #cs_nova = datos.groupby([datos.codigoSerial, datos.index.month, datos.index.day, datos.index.hour])[s_nova].first()
    cs_nova = df.groupby([df.codigoSerial, df.index.month, df.index.day, df.index.hour])[s_nova].mean()
    #cs_nova = datos.groupby(['codigoSerial']).resample('H').mean()[s_nova].values
    cs_nova.index.set_names(['codigoSerial', 'mes', 'dia', 'hora'], inplace=True)
    siata = ref.groupby([ref.index.month, ref.index.day, ref.index.hour]).pm25.first()
    siata.index.set_names(['mes', 'dia', 'hora'], inplace=True)

    detallado = pd.DataFrame(columns=['codigoSerial', 'mes', 'dia','pm25_df_hour', 'pm25_nova_hour'])
    detallado = detallado.set_index(['codigoSerial', 'mes', 'dia'])
    for i in nodos:
        for j in range(1,25):
            df_res = np.sqrt(((cs_df.loc[(i,2,j)] - siata.loc[2,j]).pow(2).sum())/(2*(cs_df.loc[(i,2,j)] + siata.loc[2,j]).count()*((cs_df.loc[(i,2,j)] + siata.loc[2,j]).mean())**2))
            nova_res = np.sqrt(((cs_nova.loc[(i,2,j)] - siata.loc[2,j]).pow(2).sum())/(2*(cs_nova.loc[(i,2,j)] + siata.loc[2,j]).count()*((cs_nova.loc[(i,2,j)] + siata.loc[2,j]).mean())**2))
            detallado.loc[(i,2,j), :] = (df_res, nova_res)
    #print(detallado)


    #detallado = detallado.reorder_levels(['codigoSerial','mes','dia','hora']).sort_index()
    detallado['cero'] = 0
    detallado['pm25_df_hour'] = detallado[['cero','pm25_df_hour']].max(axis=1, skipna=False)
    detallado['pm25_nova_hour'] = detallado[['cero','pm25_nova_hour']].max(axis=1, skipna=False)
    detallado.drop('cero', axis=1, inplace=True)

    detallado['pm25_df_day'] = detallado.groupby(level=[0,1,2]).pm25_df_hour.mean()
    detallado['pm25_nova_day'] = detallado.groupby(level=[0,1,2]).pm25_nova_hour.mean()

    detallado['pm25_df_month'] = detallado.groupby(level=[0,2]).pm25_df_day.mean()
    detallado['pm25_nova_month'] = detallado.groupby(level=[0,2]).pm25_nova_day.mean()

    dq_measure['uncer_df'] = detallado.groupby(level=[0]).pm25_df_hour.mean()
    dq_measure['uncer_nova'] = detallado.groupby(level=[0]).pm25_nova_hour.mean()
   
    # for nube in nodos:
    #     datos = df.loc[df.loc[:,'codigoSerial'] == nube]
        

    #     #incer = np.sqrt((datos.pm25_df-datos.pm25_nova).pow(2).mean()/2)/((datos.pm25_df+datos.pm25_nova).mean()/2)
    #     incer_df = np.sqrt(((cs_df - siata).pow(2).sum())/(2*(cs_df + siata).count()*((cs_df + siata).mean())**2))
    #     incer_nova = np.sqrt(((cs_nova - siata).pow(2).sum())/(2*(cs_nova + siata).count()*((cs_nova + siata).mean())**2))

    #     incer_df = max(0,1-incer_df)
    #     incer_nova = max(0,1-incer_nova)

    #     dq_measure.loc[nube,"incert_df"] = incer_df
    #     dq_measure.loc[nube,"incert_nova"] = incer_nova
        
    #     if debug == 'S':
    #         print('Nube: ', nube, '-- Incertidumbre: ', incer_df, incer_nova)

    if debug == 'S':
        print('\n')
    del i, j, df_res, nova_res, detallado
    return dq_measure

def precision(df, nodos, dq_measure, debug = 'N'):
# Cálculo de la incertidumbre del nodo con base en los valores de los sensores df y nova
# df --> Dataframe con los datos DF y nova
# nodos --> Lista con los nodos a evaluar
# dq_measure --> Dataframe en el que se alamcenan los detallados de la evaluación de la calidad del datos de cada nodo y sensor
    
    s_df, s_nova = sensors(df)
    print('precision:',s_df,s_nova)
    
    # Verifica si hay datos en el dataframe de medida de la calidad de los datos.
    if len(dq_measure) == 0:
        dq_measure = dq_df(df)
    
    if debug == 'S':
        print('******* PRECISION *********')
    
    for nube in nodos:
        datos = df.loc[df.loc[:,'codigoSerial'] == nube]

        prec_df = max(0,1 - (datos[s_df].std()/datos[s_df].mean()))
        prec_nova = max(0,1 - (datos[s_nova].std()/datos[s_nova].mean()))

        dq_measure.loc[nube,"prec_"+s_df.split('_')[1]] = prec_df
        dq_measure.loc[nube,"prec_"+s_nova.split('_')[1]] = prec_nova

        if debug == 'S':
            print('Nube: ', nube, '-- Precisión '+s_df.split('_')[1]+': ', prec_df)
            print('Nube: ', nube, '-- Precisión '+s_nova.split('_')[1]+': ', prec_nova)

    if debug == 'S':
        print('\n')
    
    try: 
        df.index.month
        dic = {s_df:{'pm25_df_hour':'[df.codigoSerial, df.index.month, df.index.day, df.index.hour]',
                            'pm25_df_day':'[df.codigoSerial, df.index.month, df.index.day]',
                            'pm25_df_month':'[df.codigoSerial, df.index.month]'},
                s_nova:{'pm25_nova_hour':'[df.codigoSerial, df.index.month, df.index.day, df.index.hour]',
                            'pm25_nova_day':'[df.codigoSerial, df.index.month, df.index.day]',
                            'pm25_nova_month':'[df.codigoSerial, df.index.month]'}}

        detallado = pd.DataFrame()

        for key1 in dic:
            for key2 in dic[key1]:
                #print(eval(dic[key1][key2]))
                media = df.groupby(eval(dic[key1][key2]))[key1].mean()
                desviacion = df.groupby(eval(dic[key1][key2]))[key1].std()
                detallado[key2] = 1-(desviacion/media)
                detallado['cero'] = 0
                detallado[key2] = detallado[['cero',key2]].max(1, skipna=False)

        detallado.drop('cero', axis=1, inplace=True)
        detallado.index.set_names(['codigoSerial', 'mes', 'dia', 'hora'], inplace=True)
        return dq_measure, detallado
   
    except :
        return dq_measure

def accuracy(datos_SIATA, datos, nodos, dq_measure, debug = 'N'):
# Cálculo de accuracy del nodo con base en los valores de los sensores df y nova en comparación con los valores de la estación SIATA de referencia
# datos_SIATA --> Dataframe con los datos de la estación SIATA de referencia
# datos --> Dataframe con los datos DF y nova
# nodos --> Lista con los nodos a evaluar
# dq_measure --> Dataframe en el que se alamcenan los detallados de la evaluación de la calidad del datos de cada nodo y sensor

    s_df, s_nova = sensors(datos)
    print('exactitud:',s_df,s_nova)

    try:
        #cs_df = datos.groupby([datos.codigoSerial, datos.index.month, datos.index.day, datos.index.hour])[s_df].first()
        cs_df = datos.groupby([datos.codigoSerial, datos.index.month, datos.index.day, datos.index.hour])[s_df].mean()
        #cs_df = datos.groupby(['codigoSerial']).resample('H').mean()[s_df].values
        cs_df.index.set_names(['codigoSerial', 'mes', 'dia', 'hora'], inplace=True)
        #cs_nova = datos.groupby([datos.codigoSerial, datos.index.month, datos.index.day, datos.index.hour])[s_nova].first()
        cs_nova = datos.groupby([datos.codigoSerial, datos.index.month, datos.index.day, datos.index.hour])[s_nova].mean()
        #cs_nova = datos.groupby(['codigoSerial']).resample('H').mean()[s_nova].values
        cs_nova.index.set_names(['codigoSerial', 'mes', 'dia', 'hora'], inplace=True)
        siata = datos_SIATA.groupby([datos_SIATA.index.month, datos_SIATA.index.day, datos_SIATA.index.hour]).pm25.first()
        siata.index.set_names(['mes', 'dia', 'hora'], inplace=True)

        detallado = pd.DataFrame()
        detallado['pm25_df_hour'] = 1-abs(cs_df - siata)/siata
        detallado['pm25_nova_hour'] = 1-abs(cs_nova - siata)/siata
        detallado = detallado.reorder_levels(['codigoSerial','mes','dia','hora']).sort_index()
        detallado['cero'] = 0
        detallado['pm25_df_hour'] = detallado[['cero','pm25_df_hour']].max(axis=1, skipna=False)
        detallado['pm25_nova_hour'] = detallado[['cero','pm25_nova_hour']].max(axis=1, skipna=False)
        detallado.drop('cero', axis=1, inplace=True)

        detallado['pm25_df_day'] = detallado.groupby(level=[0,1,2]).pm25_df_hour.mean()
        detallado['pm25_nova_day'] = detallado.groupby(level=[0,1,2]).pm25_nova_hour.mean()

        detallado['pm25_df_month'] = detallado.groupby(level=[0,3]).pm25_df_day.mean()
        detallado['pm25_nova_month'] = detallado.groupby(level=[0,3]).pm25_nova_day.mean()

        dq_measure['accu_'+s_df.split('_')[1]] = detallado.groupby(level=[0]).pm25_df_hour.mean()
        dq_measure['accu_'+s_nova.split('_')[1]] = detallado.groupby(level=[0]).pm25_nova_hour.mean()

        #print('CORTO')

        return dq_measure, detallado

    except:
        accu = pd.DataFrame(columns=['codigoSerial', 'fecha', 'hora', 'acc_df', 'acc_nova'])
        #fechas = datos_SIATA.fecha.unique().tolist()
        #horas = datos_SIATA.hora.unique().tolist()
        for fecha in [item for item in datos_SIATA.fecha.unique() if not(pd.isnull(item)) == True]:
            for hora in [item for item in datos_SIATA.hora.unique() if not(pd.isnull(item)) == True]:
                h = str(hora).split(":")

                v = datos_SIATA[str(fecha)].at_time(h[0]+':00')['pm25'][0]

                if v != 0:
                    for nube in nodos:
                        # Tomando el primer valor de la hora
                        vm_df = datos.loc[datos.loc[:,'codigoSerial'] == nube][str(fecha)].between_time(h[0]+':00', h[0]+':59')[s_df][0]
                        vm_nova = datos.loc[datos.loc[:,'codigoSerial'] == nube][str(fecha)].between_time(h[0]+':00', h[0]+':59')[s_nova][0]
                        #vm_df = datos.loc[datos.loc[:,'codigoSerial'] == nube][str(fecha)+' '+h[0]]['pm25_df'][0]
                        #vm_nova = datos.loc[datos.loc[:,'codigoSerial'] == nube][str(fecha)+' '+h[0]]['pm25_nova'][0]

                        # Tomando el promedio de la hora
                        #vm_df = datos.loc[datos.loc[:,'codigoSerial'] == nube][str(fecha)+' '+h[0]]['pm25_df'].mean()
                        #vm_nova = datos.loc[datos.loc[:,'codigoSerial'] == nube][str(fecha)+' '+h[0]]['pm25_nova'].mean()

                        if not np.isnan(vm_df):
                            accur_df = max(0,1-(abs(vm_df-v)/v))
                        else:
                            accur_df = np.NaN
                        
                        if not np.isnan(vm_nova):
                            accur_nova = max(0,1-(abs(vm_nova-v)/v))
                        else:
                            accur_nova = np.NaN
                        

                        if debug == 'S':
                            if (accur_df < 0) or (accur_nova < 0):
                                print(nube, fecha, hora, accur_df, accur_nova, '*****',vm_df, vm_nova, v)

                        accu=accu.append({'codigoSerial': nube,
                                                    'fecha': fecha, 
                                                    'hora': hora, 
                                                    'exac_df': accur_df, 
                                                    'exac_nova': accur_nova} , ignore_index=True)
        
        print('LARGO')

        for nube in nodos:
            filtro = accu.loc[accu.loc[:,'codigoSerial'] == nube]
            dq_measure.loc[nube,"accu_"+s_df.split('_')[1]] = filtro.acc_df.mean()
            dq_measure.loc[nube,"accu_"+s_nova.split('_')[1]] = filtro.acc_nova.mean()

        return dq_measure

    
def outliers(datos, outliers_detected, dq_measure):
# Cálculo de la cantidad y porcentaje de outliers detectados para cada nodo
# datos --> Dataframe con los datos DF y nova
# outliers_detected --> Diccionario con los índices de los outliers detectados en cada nodo y cada sensor
# dq_measure --> Dataframe en el que se alamcenan los detallados de la evaluación de la calidad del datos de cada nodo y sensor

    for nube in outliers_detected:
        for sensor in outliers_detected[nube]:

            cant = datos.loc[datos.loc[:,'codigoSerial'] == nube][sensor].count()
            outliers = len(outliers_detected[nube][sensor])

            dq_measure.loc[nube,"cant"] = cant
            dq_measure.loc[nube,"outl_"+sensor.split('_')[1]] = outliers
            dq_measure.loc[nube,"outl_"+sensor.split('_')[1]+"_%"] = outliers/cant
    
    return dq_measure

def correlacion(df, nodos, dq_measure, debug = 'N'):
# Basado en https://www.cienciadedatos.net/documentos/pystats05-correlacion-lineal-python.html
# Calcula la correlación de los valores de DF y Nova, usando pearson (normalidad) y Jackknife (no paramétrica).
# Pearson la afecta mucho los outliers, jackknife atenua un poco la presencia de outliers.

# df --> Dataframe con los datos DF y nova
# nodos --> Lista con los nodos a evaluar
# dq_measure --> Dataframe en el que se alamcenan los detallados de la evaluación de la calidad del datos de cada nodo y sensor

        s_df, s_nova = sensors(df)
        print('correlacion:',s_df,s_nova)

    #try:
        detallado = pd.DataFrame()
        # Referencia: https://stackoverflow.com/questions/28988627/pandas-correlation-groupby
        detallado['pm25_df_hour'] = detallado['pm25_nova_hour'] = df.groupby([df.codigoSerial, df.index.month, df.index.day, df.index.hour])[[s_df, s_nova]].corr().unstack().iloc[:,1]
        detallado['pm25_df_day'] = detallado['pm25_nova_day'] = df.groupby([df.codigoSerial, df.index.month, df.index.day])[[s_df, s_nova]].corr().unstack().iloc[:,1]
        detallado['pm25_df_month'] = detallado['pm25_nova_month'] = df.groupby([df.codigoSerial, df.index.month])[[s_df, s_nova]].corr().unstack().iloc[:,1]
        detallado.index.set_names(['codigoSerial', 'mes', 'dia', 'hora'], inplace=True)

        dq_measure['concord'] = df.groupby([df.codigoSerial])[[s_df, s_nova]].corr().unstack().iloc[:,1]
        
        return dq_measure, detallado

    #except:
    # Verifica si hay datos en el dataframe de medida de la calidad de los datos.
        print('Hubo error')
        if len(dq_measure) == 0:
            dq_measure = dq_df(df)

        if debug == 'S':
            print('******* CORRELACIÓN *********')
        
        for nube in nodos:
            datos = df.loc[df.loc[:,'codigoSerial'] == nube]
            datos.reset_index(inplace=True, drop=True)

            # Correlación de Pearson
            pm25_df = datos['pm25_df'][datos['pm25_df'].notna()]
            pm25_nova = datos['pm25_nova'][datos['pm25_nova'].notna()]
            r, p = stats.pearsonr(pm25_df, pm25_nova)

            # Correlación de Jackknife
            #correlacion = correlacion_jackknife(np.array(pm25_df), np.array(pm25_nova))

            dq_measure.loc[nube,"corr_p"] = r
            #dq_measure.loc[nube,"corr_j"] = correlacion['promedio']

            if debug == 'S':
                print('Nube: ', nube, '-- Correlación Pearson: ', r, 'valor-p: ', p)
                print('Nube: ', nube, '-- Correlación Jackknife: ', correlacion['promedio'], 'Error estándar: ', correlacion['se'])

        if debug == 'S':
            print('\n')
        
        del nube, datos, r, p#, correlacion
        return dq_measure

def correlacion2(df, ref, nodos, dq_measure, debug = 'N'):
# Basado en https://www.cienciadedatos.net/documentos/pystats05-correlacion-lineal-python.html
# Calcula la correlación de los valores de DF y Nova, usando pearson (normalidad) y Jackknife (no paramétrica).
# Pearson la afecta mucho los outliers, jackknife atenua un poco la presencia de outliers.

# df --> Dataframe con los datos DF y nova
# ref --> Dataframe con los datos del nodo de referencia, en este caso, los datos de la estación SIATA de referencia
# nodos --> Lista con los nodos a evaluar
# dq_measure --> Dataframe en el que se alamcenan los detallados de la evaluación de la calidad del datos de cada nodo y sensor

        s_df, s_nova = sensors(df)
        print('correlacion2:',s_df,s_nova)

        #cs_df = datos.groupby([datos.codigoSerial, datos.index.month, datos.index.day, datos.index.hour])[s_df].first()
        cs_df = df.groupby([df.codigoSerial, df.index.month, df.index.day, df.index.hour])[s_df, s_nova].mean()
        #cs_df = datos.groupby(['codigoSerial']).resample('H').mean()[s_df].values
        # cs_df.index.set_names(['codigoSerial', 'mes', 'dia', 'hora'], inplace=True)
        # #cs_nova = datos.groupby([datos.codigoSerial, datos.index.month, datos.index.day, datos.index.hour])[s_nova].first()
        # cs_nova = df.groupby([df.codigoSerial, df.index.month, df.index.day, df.index.hour])[s_nova].mean()
        # #cs_nova = datos.groupby(['codigoSerial']).resample('H').mean()[s_nova].values
        # cs_nova.index.set_names(['codigoSerial', 'mes', 'dia', 'hora'], inplace=True)
        siata = ref.groupby([ref.index.month, ref.index.day, ref.index.hour]).pm25.first()
        siata.index.set_names(['mes', 'dia', 'hora'], inplace=True)


    #try:
        detallado = pd.DataFrame()
        # Referencia: https://stackoverflow.com/questions/28988627/pandas-correlation-groupby
        detallado['pm25_df_hour'] = detallado['pm25_nova_hour'] = df.groupby([df.codigoSerial, df.index.month, df.index.day, df.index.hour])[[s_df, s_nova]].corr().unstack().iloc[:,1]
        detallado['pm25_df_day'] = detallado['pm25_nova_day'] = df.groupby([df.codigoSerial, df.index.month, df.index.day])[[s_df, s_nova]].corr().unstack().iloc[:,1]
        detallado['pm25_df_month'] = detallado['pm25_nova_month'] = df.groupby([df.codigoSerial, df.index.month])[[s_df, s_nova]].corr().unstack().iloc[:,1]
        detallado.index.set_names(['codigoSerial', 'mes', 'dia', 'hora'], inplace=True)

        dq_measure['corr'] = df.groupby([df.codigoSerial])[[s_df, s_nova]].corr().unstack().iloc[:,1]
        
        return dq_measure, detallado

def accuracy2(referencia, datos):
# Cálculo de accuracy del nodo con base en los valores de los sensores df y nova en comparación con los valores de la estación SIATA de referencia
# datos_SIATA --> Dataframe con los datos de la estación SIATA de referencia
# datos --> Dataframe con los datos DF y nova
# nodos --> Lista con los nodos a evaluar
# dq_measure --> Dataframe en el que se alamcenan los detallados de la evaluación de la calidad del datos de cada nodo y sensor

    
    feat = list(datos.columns)
    if 'codigoSerial' in feat:
        feat.remove('codigoSerial')
    col = list(referencia.columns)[0]
    ref = referencia.groupby([referencia.index.month, referencia.index.day, referencia.index.hour])[col].first()
    ref.index.set_names(['mes', 'dia', 'hora'], inplace=True)

    detallado = pd.DataFrame()
    dq_measure = pd.DataFrame()

    for f in feat:
        hour = str(f)+'_hour'
        day = str(f)+'_day'
        month = str(f)+'_month'
        acc = datos.groupby([datos.codigoSerial, datos.index.month, datos.index.day, datos.index.hour])[f].first()
        acc.index.set_names(['codigoSerial', 'mes', 'dia', 'hora'], inplace=True)
        
        detallado[hour] = 1-abs(acc - ref)/ref
        detallado['cero'] = 0
        detallado[hour] = detallado[['cero',hour]].max(axis=1, skipna=False)
        detallado.drop('cero', axis=1, inplace=True)

        detallado[day] = detallado.groupby(level=[0,1,2])[hour].mean()
        detallado[month] = detallado.groupby(level=[0,3])[day].mean()

        dq_measure[f] = detallado.groupby(level=[3])[hour].mean()
    
    detallado = detallado.reorder_levels(['codigoSerial','mes','dia','hora']).sort_index()
        
    return dq_measure

# Función Jackknife correlation
# ==============================================================================
# Tomado de: https://www.cienciadedatos.net/documentos/pystats05-correlacion-lineal-python.html

def correlacion_jackknife(x, y):
    '''
    Esta función aplica el método de Jackknife para el cálculo del coeficiente
    de correlación de Pearson.
    
    
    Parameters
    ----------
    x : 1D np.ndarray, pd.Series 
        Variable X.
        
    y : 1D np.ndarray, pd.Series
        Variable y.     

    Returns 
    -------
    correlaciones: 1D np.ndarray
        Valor de correlación para cada iteración de Jackknife
    '''
    
    n = len(x)
    valores_jackknife = np.full(shape=n, fill_value=np.nan, dtype=float)
    
    for i in range(n):
        # Loop para excluir cada observación y calcular la correlación
        r = stats.pearsonr(np.delete(x, i), np.delete(y, i))[0]
        valores_jackknife[i] = r

    promedio_jackknife = np.nanmean(valores_jackknife)
    standar_error = np.sqrt(((n - 1) / n) * \
                    np.nansum((valores_jackknife - promedio_jackknife) ** 2))
    bias = (n - 1) * (promedio_jackknife - stats.pearsonr(x, y)[0])
    
    detallados = {
        'valores_jackknife' : valores_jackknife,
        'promedio'          : promedio_jackknife,
        'se'                : standar_error,
        'bias'              : bias
    }
    
    del n, valores_jackknife, r, promedio_jackknife, standar_error, bias
    return detallados

def DQ_Index(dq_measure):

    Waccuracy        = 0.44633818125
    Wuncertainty     = 0.23942839515
    Wcompleteness    = 0.18851638716
    Wprecision       = 0.12571703644

    WaccuracyEPA        = 0.295081967
    WuncertaintyEPA     = 0.163934426
    WcompletenessEPA    = 0.245901639
    WprecisionEPA       = 0.295081967

    dq_measure['DQ_Index'] = Waccuracy * dq_measure[['accu_df', 'accu_nova']].mean(axis=1) + \
                                Wuncertainty * dq_measure['uncer'] + \
                                Wcompleteness * dq_measure[['comp_df', 'comp_nova']].mean(axis=1) + \
                                Wprecision * dq_measure[['prec_df', 'prec_nova']].mean(axis=1)
    
    #dq_measure['DQ_Index_EPA'] = WaccuracyEPA * dq_measure[['exac_df', 'exac_nova']].mean(axis=1) + \
    #                            WuncertaintyEPA * dq_measure['incert'] + \
    #                            WcompletenessEPA * dq_measure[['comp_df', 'comp_nova']].mean(axis=1) + \
    #                            WprecisionEPA * dq_measure[['prec_df', 'prec_nova']].mean(axis=1)
    
    dq_measure['Codigo_EPA'] = ''
    try:
        dq_measure.loc[dq_measure[['comp_df', 'comp_nova']].mean(axis=1) > 0.75, 'Codigo_EPA'] = dq_measure[['Codigo_EPA']]+'C'
    except:
        None

    try:
        dq_measure.loc[dq_measure['uncer'] >= 0.5, 'Codigo_EPA'] = dq_measure[['Codigo_EPA']]+'U'
    except:
        None
    
    try:
        dq_measure.loc[dq_measure[['prec_df', 'prec_nova']].mean(axis=1) >= 0.9, 'Codigo_EPA'] = dq_measure[['Codigo_EPA']]+'P'
    except:
        None

    try:
        dq_measure.loc[dq_measure[['accu_df', 'exac_nova']].mean(axis=1) >= 0.9, 'Codigo_EPA'] = dq_measure[['Codigo_EPA']]+'A'
    except:
        None

    nodos = dq_measure[dq_measure['Codigo_EPA'].str.contains('C')].index.tolist()

    
    return dq_measure, nodos


def DQ_Index2(dq_measure):

    ext = []
    for c in dq_measure.columns[:8]:
        c1 = c.split('_')
        if (len(c1) > 1) and (c1[1] not in ext):
            ext.append(c1[1])

    DQ_nodes = {}
    for nodo in dq_measure.index:
        DQ_sensor = {}
        for sensor in ext: #['df','nova']:
            DQ_dim = {}
            for c in dq_measure.columns[:8]:
                c1 = c.split('_')
                if len(set(ext) & set(c1)) > 0:
                    if sensor in c1:
                        DQ_dim[c1[0]] = dq_measure.loc[nodo][c]
                else:
                    DQ_dim[c1[0]] = dq_measure.loc[nodo][c]

            
            DQ_sensor[sensor] = DQ_dim
        
        DQ_nodes[str(nodo)] = DQ_sensor
    
    data = np.array([[1.0, 5.0, 5.0, 3.0, 1.0, 0.3, 5.0, 1.0],
                 [0.2, 1.0, 0.2, 0.3, 1.0, 0.3, 5.0, 1.0],
                 [0.2, 5.0, 1.0, 1.0, 1.0, 3.0, 7.0, 1.0],
                 [0.3, 3.0, 1.0, 1.0, 0.3, 1.0, 3.0, 1.0],
                 [1.0, 1.0, 1.0, 3.0, 1.0, 1.0, 5.0, 0.2],
                 [3.0, 3.0, 0.3, 1.0, 1.0, 1.0, 7.0, 1.0],
                 [0.2, 0.2, 0.1, 0.3, 0.2, 0.1, 1.0, 0.2],
                 [1.0, 1.0, 1.0, 1.0, 5.0, 1.0, 5.0, 1.0]])

    variables = ['accu', 'prec', 'uncer', 'comp', 'time', 'volum', 'redund', 'concord']

    pwM = pd.DataFrame(data, columns=variables, index=variables)



    ######  PARA EL ARTÍCULO SE DEJAN SOLO ACCURACY, PRECISION, COMPLETENESS, CONCORDANCE Y CONFIDENCE COMO OBJETIVO DE LA UNCERTAINTY ################

    pwM.drop(['time', 'volum', 'redund'], axis=1, inplace=True)
    pwM.drop(['time', 'volum', 'redund'], axis=0, inplace=True)

    pwMNor =pd.DataFrame(columns=pwM.columns, index=pwM.index) 

    for i in pwM.columns:
        for j in pwM.index:
            pwMNor.loc[i,j] = pwM.loc[i,j]/pwM.loc[:,j].sum()

    weights = {}

    for i in pwMNor.index:
        weights[i] = pwMNor.loc[i,:].mean()
    
    #'***************DQ Index de los nodos***************')
    for n in DQ_nodes.keys():
        for s in DQ_nodes[n].keys():
            DQ_index = 0
            for d in DQ_nodes[n][s].keys():
                #print(n,s,d, DQ_nodes[n][s][d], weights[d])
                DQ_index += DQ_nodes[n][s][d]*weights[d]
    
            
            dq_measure.loc[int(n),"DQ_Index_"+s] = DQ_index

    dq_measure['Codigo_EPA'] = ''
    try:
        dq_measure.loc[dq_measure[['comp_'+ext[0], 'comp_'+ext[1]]].mean(axis=1) > 0.75, 'Codigo_EPA'] = dq_measure[['Codigo_EPA']]+'C'
    except:
        None

    try:
        dq_measure.loc[dq_measure['uncer'] >= 0.5, 'Codigo_EPA'] = dq_measure[['Codigo_EPA']]+'U'
    except:
        None
    
    try:
        dq_measure.loc[dq_measure[['prec_'+ext[0], 'prec_'+ext[1]]].mean(axis=1) >= 0.9, 'Codigo_EPA'] = dq_measure[['Codigo_EPA']]+'P'
    except:
        None

    try:
        dq_measure.loc[dq_measure[['accu_'+ext[0], 'exac_'+ext[1]]].mean(axis=1) >= 0.9, 'Codigo_EPA'] = dq_measure[['Codigo_EPA']]+'A'
    except:
        None

    nodos = dq_measure[dq_measure['Codigo_EPA'].str.contains('C')].index.tolist()


    
    return dq_measure, nodos
    