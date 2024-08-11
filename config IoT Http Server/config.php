<?php

date_default_timezone_set('Asia/Jakarta');

$database = (object) [
    'host' => 'localhost',
    'user' => 'is4ac',
    'password' => 'Is4aC_123',
    'db' => 'is4ac_local',
    'port' => 3306,
];

$conn = new mysqli(
    $database->host,
    $database->user,
    $database->password,
    $database->db,
    $database->port
);

if ($conn->connect_error)
    die("Connection failed" . $conn->connect_error);
