# BELOM Knob JS Port

Этот каталог содержит отдельный JS-порт интерфейса из проекта `Touch-Knob-Belom`, собранный на основе `react-example`.

## Что это

- Отдельное web-приложение (`React + Vite + Tailwind v4`) для быстрого UI-прототипирования.
- Базовая логика экранов и настроек перенесена в JS-компоненты из `react-example/src/app`.
- Подходит для визуальной отладки и согласования анимаций/стилей до переноса в LVGL C.

## Что это не

- Это не прошивка для ESP32.
- Аппаратные части (`knob`, `button`, `PWM`, `NVS`, `LVGL C`) здесь не исполняются, только симуляция UI.

## Запуск

```bash
cd belom-knob-js
npm install
npm run dev
```

Открыть URL, который покажет Vite (обычно `http://localhost:5173`).

## Сборка

```bash
npm run build
npm run preview
```

## Структура

- `src/app/App.tsx` — основной экран и state-машина прототипа.
- `src/app/components/SettingsScreen.tsx` — настройки (включая sensitivity UI).
- `src/main.tsx` — web entrypoint.
- `src/styles/*` — стили и темы.

## Дальше

Если нужно, следующим шагом можно вынести бизнес-логику в общий слой:
- `core/state.ts` (единый state/события для C и JS),
- `core/reducer.ts` (действия энкодера и кнопки),
- адаптеры рендера: `lvgl` и `react`.
