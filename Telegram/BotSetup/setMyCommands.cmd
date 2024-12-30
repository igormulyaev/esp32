@chcp 65001 > nul
@call secret_var.cmd

curl -H "Content-Type: application/json" -d "{\"commands\":[{\"command\":\"start\",\"description\":\"Старт\"},{\"command\":\"help\",\"description\":\"Помощь\"}]}" %boturl%/setMyCommands
