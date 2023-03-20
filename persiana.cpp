/*#include "persiana.h"
#include <Ticker.h>

Persiana::Persiana()
{

}


void Persiana::sube()
{
  if(_estado==ssubiedo)return;
    else if(_estado==ssbajando || estado==sstop) _posicion=calculaPosicion();
      _lastTime=now();
      _estado=ssubiedo;
      //envia orden
      Persiana::fin.once(_tmaximo,Persiana::callBack);
    
  
}

void Persiana::baja()
{
   if(_estado==ssbajando )return;
    else if(_estado==ssubiedo || estado==sstop) _posicion=calculaPosicion();
      _lastTime=now();
      _estado=ssbajando;
      //envia orden
      Persiana::fin.once(_tmaximo,Persiana::callBack);
}

void Persiana::stop()
{
       if(_estado==sstop  )return;
        else if(_estado==ssubiedo || estado==ssbajando) _posicion=calculaPosicion();
          _lastTime=now();
          _estado=sstop;
          //envia orden
          Persiana::fin.once(_tmaximo,Persiana::callBack);
}

int Persiana::posicion()
{
      return _posicion;
}

void Persiana::callBack()
{
      _estado=sparado;
}

int Persiana::calculaPosicion()
{
      long tiempo=now()-_lastTime;
      int cambio=tiempo*VELOCIDADPERSIANA; //quizas la velocidad deberia ser independiente
      _posicion+=_(estado==ssubiedo)?cambio:-cambio;
}
*/
