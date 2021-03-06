# Test task for the autumn Jet-Brains project "Deadlock detector in the debugger" 

Developed a safe-mutex -- a wrapper over the std::mutex that can allow deadlocks to be defined as they happen. How does it work: at any time the program maintains a
directed graph in which the vertexes are threads, the existence of an edge A -> B is equivalent to the fact that thread A is holding a mutex, which is blocking thread B. The existence of a cycle in such a graph is equivalent to the existence of a deadlock in the program.

## project's building

```
git clone https://github.com/fleex-x/JB-deadlock-detector-test-task.git
cd JB-deadlock-detector-test-task
cmake .
make
./safe-mutex-tests
./deadlock-main
```



# Тестовое задание для проекта "Детектор взаимных блокировок (deadlock) в отладчике"

1. [Сборка](#Сборка)
1. [Реализация](#Реализация)
1. [Note1](#Note1)
1. [Note2](#Note2)


## Сборка

```
git clone https://github.com/fleex-x/JB-deadlock-detector-test-task.git
cd JB-deadlock-detector-test-task
cmake .
make
./safe-mutex-tests
./deadlock-main
```

Тесты могут работать долго (от 20 секунд), так как там очень много операций создания потока/блокировки мьютекса. Исполняемый файл deadlock-main демонстрирует, что при возникновении взаимной блокировки решение аварийно завершается.

## Реализация
Возникновение взаимной блокировки определяется следующим образом: в настоящий момент времени некоторые потоки находятся в состоянии ожидания мьютекса, который блокируется другим потоком. В итоге создается граф зависимости потоков, где существование направленного ребра между потоками  равносильно тому, что один поток ожиадет мьютекс, который блокируется другим потоком. При этом в настоящий момент возникла взаимная блокировка равносильно тому, что в этом графе есть цикл. В итоге в этом графе запускается поиск цикла, таким образом и происходит проверка на взаимную блокировку.

## Note1
Эти примечания относятся к строчкам, где есть комментарии `check Note1` и `check Note2`.
В этих строчках есть небольшая проблема: операции взятия мьютекса и обновления графа не атомарны. То есть возможна такая ситуация: мьютекс был взят, информация в графе не обновлена, между этими событиями в граф добавились еще ребра. Могло ли произойти так, что из-за того, что в графе осталось старое ребро, мы нашли deadlock, которого на самом деле нет? Такого произойти не могло по следующей причине: перед тем, как мьютекс был отпущен, было отмечено, что мьютекс теперь свободный. Значит и сейчас в `thread_dependencies` отмечено, что мьютекс свободный. А значит этого ребра в графе нет. (потому что ребро строится из `current_thread` и `current_expectation`).

## Note2
Здесь могла произойти другая ситуация: в графе отмечено, что мьютекс отпущен, но в это время в граф добавились еще ребра, был запущен поиск цикла и не был обнаружен deadlock, который на самом деле был. Эта ситуация не могла произойти по следующей причине: если бы был deadlock связанный с текущим потоком, то это значит, что текущий поток находится в ожидании взятия мьютекса. А мы точно знаем, что он сейчас не в таком состоянии, поэтому deadlock'a тут быть не могло.
