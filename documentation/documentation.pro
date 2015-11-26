DOTSOURCES += \
    graphs/complex.dot \
    graphs/newMainLoop.gv \
    graphs/CounterScheme.gv \
    graphs/HVScheme.gv \
    graphs/MainLoop.gv


#Добавление генерации
dot.output = $${OUT_PWD}/${QMAKE_FILE_BASE}.cpp
dot.commands += printf \"\" >> $${OUT_PWD}/${QMAKE_FILE_BASE}.cpp &&
dot.commands += dot -Tpng ${QMAKE_FILE_NAME} -o $${PWD}/img/${QMAKE_FILE_BASE}.png
dot.input = DOTSOURCES
dot.variable_out = SOURCES

QMAKE_EXTRA_COMPILERS += dot

SOURCES += \
    main.cpp

