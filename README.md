# IO PAMI

Programme pour gérer les IO des PAMI Terraforming Mars 2024 sur une Arduino Nano en I2C.

- 2 servos
- 3 entrées numériques
- 3 entrées analogiques

## Protocole

### Requete

#### Changer la position des servos

`S<valeur servo1><valeur servo2>`

Les valeurs sont des entiers signés sur deux octets.

#### Afficher les valeurs des inputs (debug Serial uniquement)

`F`

### Réponse

`<valeur d1><valeur d2><valeur d3><valeur a1><valeur a2><valeur a3>`

Valeurs numériques (0 ou 1) sur un octet, valeurs analogiques (entiers signés) sur deux octets.
