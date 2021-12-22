Ettore Ricci
# **Relazione**
## Livello trasporto
Ho scelto il protocollo TCP perché per un’applicazione di messaggistica è preferibile l’affidabilità al throughput
## Livello applicazione
Ogni pacchetto è composto da un header e da un payload

- Header:
    |1B|4B|
    | :- | :- |
    |Tipo del messaggio|Lunghezza del payload|

- Payload: cambia struttura a seconda del tipo del messaggio, questa è documentata nel dettaglio in [global.d/network\_tools/network\_tools.h](global.d/network\_tools/network\_tools.h)
## Gestione delle richieste lato server
Il server gestisce le richieste in modo **sequenziale** con *select*, ho fatto questa scelta per evitare la complessità che deriva dalla programmazione concorrente e per facilitare la condivisione delle strutture dati comuni a tutte le connessioni.

Per evitare che il server rimanga in idle attendendo un intero messaggio da un device lento, il contenuto del messaggio viene bufferizzato in una struttura globale e utilizzato solo quando si riceve l’ultimo byte.

