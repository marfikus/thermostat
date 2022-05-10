
# Простой термостат.

Устройство позволяет более-менее поддерживать заданную температуру.  
В качестве датчика в данном случае используется терморезистор на 100 кОм (при повышении температуры сопротивление понижается). Точная маркировка неизвестна, но опытным путём был определён достаточно линейный диапазон от 27 до 100 градусов Цельсия, и далее под него производится расчёт.  
Регулятор - любой подходящий переменный резистор.  
В качестве мозга можно использовать Arduino или подходящий МК. В коде уже есть конфигурации под Arduino и под ATtiny13A (нужное раскомментировать, ненужное закомментировать).

В самом простом варианте (**_thermostat_simple_**) алгоритм работает следующим образом:  
Считывается значение датчика. Если оно меньше минимально возможного, то считаем датчик в обрыве, о чём будет сигнализировать светодиод (будет мигать примерно 2 раза в секунду). Если значение в норме, то считывается значение регулятора, преобразуется к диапазону датчика и сравнивается с его значением. Если значение датчика с учётом заданного гистерезиса больше значения регулятора, то увеличиваем счётчик срабатываний. Как только этот счётчик достигнет заданного максимального количества срабатываний, то выполняется выключение нагревателя и сброс счётчика. Аналогично выполняются обратные действия.  
Гистерезис позволяет установить некую переходную зону около заданной температуры и тем самым снизить количество переключений.
Счётчики позволяют избежать ложных срабатываний и тоже, в общем, работают как гистерезис. Экспериментируя, можно подобрать оптимальные значения для конкретной ситуации.

В варианте прошивки **_thermostat_with_adjust_up_limit_** дополнительно реализована возможность настройки верхней границы температурного диапазона при помощи подстроечного резистора (`UP_LIMIT_ADJUSTER`). В остальном всё аналогично **_thermostat_simple_**.

В варианте прошивки **_thermostat_without_sparking_on_relay_**, который на данный момент я реально использую, немного усложнена схема: в цепь управления нагревателем добавлен симистор, параллельно реле. При включении нагревателя сначала открывается симистор, а через некоторое время замыкается реле (то есть оно шунтирует симистор). Отключение происходит в обратном порядке. Это позволяет защитить контакты реле от искрообразования, а также избежать постоянного нагрева симистора в том случае, если бы он один коммутировал нагреватель (в текущем виде симистор работает только во время включения и отключения нагревателя).  
Такой вариант есть смысл делать только при большом коммутируемом токе (в моём случае это около 7А). Остальная логика работает также.

Электрические схемы:  
Простой вариант (**_thermostat_simple_**):
![schematic_thermostat_simple](/schematic_thermostat_simple.png "schematic_thermostat_simple.png")

С регулировкой верхней границы температурного диапазона (**_thermostat_with_adjust_up_limit_**):
![schematic_thermostat_with_adjust_up_limit](/schematic_thermostat_with_adjust_up_limit.png "schematic_thermostat_with_adjust_up_limit.png")

С симистором (**_thermostat_without_sparking_on_relay_**):
![schematic_thermostat_without_sparking_on_relay](/schematic_thermostat_without_sparking_on_relay.png "schematic_thermostat_without_sparking_on_relay.png")
