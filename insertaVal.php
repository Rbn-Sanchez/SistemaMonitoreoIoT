<?php
// Autor: Ruben Sanchez Mayen y Lorena Palomino Castillo
// Lee los valores de la peticion y los graba en la BD
include 'conexion.php';
if($_GET){  
    
    date_default_timezone_set("America/Mexico_City");
    $fecha = date("Y-m-d");
    $hora = date("h:i:sa");

    // Estos datos los vamos a recibir desde una página
    $idPersona = $_GET["idPersona"];
    $valorTemp = $_GET["valorTemp"];
    $valorPulso = $_GET["valorPulso"];
    $valorSpo2 = $_GET["valorSpo2"];

    // Validar temperatura
    if($valorTemp != 0){
        $sql_agregar = "INSERT INTO temperatura (fecha,hora,valor,idPersona) 
        VALUES (?,?,?,?)";

        $sentencia_agregar = $pdo->prepare($sql_agregar);
        $resultado = $sentencia_agregar->execute(array($fecha,$hora,$valorTemp,$idPersona));

        if ($resultado==true){
            $sentencia_agregar = null;
            echo "\nSe inserto la temperatura de manera correcta";
        } else {
            echo "\nError al insertar temperatura en la BD";
        }
    }

    // validar Pulso
    if($valorPulso != 0){
        $sql_agregar = "INSERT INTO pulso (fecha,hora,valor,idPersona) 
        VALUES (?,?,?,?)";

        $sentencia_agregar = $pdo->prepare($sql_agregar);
        $resultado = $sentencia_agregar->execute(array($fecha,$hora,$valorPulso,$idPersona));

        if ($resultado==true){
            $sentencia_agregar = null;
            echo "\nSe inserto el pulso de manera correcta";
        } else {
            echo "\nError al insertar pulso en la BD";
        }
    }

    // validar Spo2
    if($valorSpo2 != 0){
        $sql_agregar = "INSERT INTO oximetria (fecha,hora,valor,idPersona) 
        VALUES (?,?,?,?)";

        $sentencia_agregar = $pdo->prepare($sql_agregar);
        $resultado = $sentencia_agregar->execute(array($fecha,$hora,$valorSpo2,$idPersona));

        if ($resultado==true){
            $sentencia_agregar = null;
            echo "\nSe inserto la oximetría de manera correcta";
        } else {
            echo "\nError al insertar oximetría en la BD";
        }
    }
    // Estos datos los vamos a recibir desde una página
    /*$idPersona = $_GET["idPersona"];
    $valor = $_GET["valor"];

    $sql_agregar = "INSERT INTO temperatura (fecha,hora,valor,idPersona) 
        VALUES (?,?,?,?)";

    $sentencia_agregar = $pdo->prepare($sql_agregar);
    $resultado = $sentencia_agregar->execute(array($fecha,$hora,$valor,$idPersona));

    if ($resultado==true){
        $sentencia_agregar = null;
        $pdo = null;
        echo "\nSe insertaron los datos de manera correcta";
    } else {
        echo "Error al insertar temperatura en la BD";
    }*/
    echo "\nfinalizado";
    $pdo = null;
} else {
    //No hay informacion en el URL
    echo "Faltan los datos";
}
?>