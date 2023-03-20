/*#ifndef PERSIANA
#define PERSIANA
#define VELOCIDADPERSIANA 23 //porcentaje por msegundo
#include <String.h>
#include <Ticker.h>

class Persiana {

  public:
      typedef enum
    {
      sparado=0,
      sstop=1,
      ssubiedo=2,
      sbajando=3
      
    } persianaStates;
    
    Persiana();
    void sube();
    void baja();
    void stop();
    int posicion();
    void callBack();
    

   private:
    Ticker fin;
    int _posicion;
    int _estado;
    int _tmaximo; //tiempo maximo que tarda en bajar milisegundos
    unsigned long  _lastTime;
    int calculaPosicion();
#endif*/
