# IO PAMI

Programme pour gérer les IO des PAMI Terraforming Mars 2024 sur une Arduino Nano en I2C.

- 2 servos
- 3 entrées numériques (pullup)
- 3 GP2D12

## Protocole

### Requete

#### Changer la position des servos

`S<valeur servo1><valeur servo2>`

Les valeurs (microsecondes) sont des entiers signés sur deux octets.

#### Arrêter de piloter les servos

`D`

#### Afficher les valeurs des inputs (debug Serial uniquement)

`F`

### Réponse

`<input1><input2><input3><gp2d1><gp2d2><gp2d3>`

Les valeurs numériques (0 ou 1) et les valeurs des GP2D (distance) sont sur un octet.
