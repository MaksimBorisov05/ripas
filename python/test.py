import numpy as np
import matplotlib.pyplot as plt
from scipy.interpolate import make_interp_spline

# Ввод данных
x = np.array([0.1, 0.2, 0.5, 1, 5])
y = np.array([0.35, 0.6, 0.8, 1.4, 0.02])

# Степень полинома (можешь менять: 2, 3, 4...)
degree = 3

# Аппроксимация
coeffs = np.polyfit(x, y, degree)
poly = np.poly1d(coeffs)

# Генерация гладкой кривой
x_smooth = np.linspace(min(x), max(x), 500)
spline = make_interp_spline(x, y, k=3)  # k=3 — кубический сплайн
y_smooth = spline(x_smooth)

# Построение графика
plt.figure(figsize=(10, 6))

# Точки
plt.scatter(x, y, label='Экспериментальные точки')

# Аппроксимирующая кривая
plt.plot(x_smooth, y_smooth, label=f'Полином {degree}-й степени')

#plt.xscale('log')

# Оформление
plt.title('Вариант 2')
plt.xlabel('f,кГц')
plt.ylabel('фи')
plt.grid()
plt.legend()

plt.show()