# OTA Updates

Система автообновлений для форков Telegram Desktop через GitHub-репозиторий.

Клиент периодически проверяет JSON-манифест `current4` по настроенному URL. Если доступна новая версия — скачивает подписанный пакет обновления и устанавливает его.

---

## Как это работает

1. Клиент запрашивает `{UPDATE_URL}/current4` — JSON-файл с версиями для каждой платформы.
2. Если доступна новая версия — скачивает файл обновления (напр. `tx64upd6005002`) по `{UPDATE_URL}/{link}`.
3. Файл подписан RSA-ключом (1024-bit), клиент проверяет подпись перед установкой.
4. После проверки обновление распаковывается (LZMA) и применяется через `Updater.exe`.

`UPDATE_URL` задаётся в `Telegram/SourceFiles/storage/localstorage.cpp` в функции `readAutoupdatePrefixRaw()`.

---

## Настройка для своего форка

### Файлы, которые нужно изменить

| Файл | Что менять |
|------|------------|
| `storage/localstorage.cpp` | URL вашего репо обновлений в `readAutoupdatePrefixRaw()` |
| `config.h` | `UpdatesPublicKey` / `UpdatesPublicBetaKey` — ваши RSA-публичные ключи |
| `_other/packer.cpp` | `PublicKey` / `PublicBetaKey` — те же публичные ключи (дубль для Packer) |
| `DesktopPrivate/packer_private.h` | `PrivateKey` / `PrivateBetaKey` — ваши RSA-приватные ключи |
| `DesktopPrivate/alpha_private.h` | `AlphaPrivateKey` — приватный ключ для альфа-версий |
| `mtproto/mtp_instance.cpp` | Закомментирован `autoupdate_url_prefix` из Telegram API |
| `core/update_checker.cpp` | Отключен MTP checker (обновления через каналы Telegram) |

### Генерация RSA-ключей

Нужны две пары (stable + beta) и одна для alpha. Размер ключа — **1024 bit** (совместимость с форматом Packer).

```python
# pip install cryptography
from cryptography.hazmat.primitives.asymmetric import rsa
from cryptography.hazmat.primitives import serialization

key = rsa.generate_private_key(public_exponent=65537, key_size=1024)

# Публичный ключ → config.h, packer.cpp
pub = key.public_key().public_bytes(
    encoding=serialization.Encoding.PEM,
    format=serialization.PublicFormat.PKCS1
).decode()

# Приватный ключ → DesktopPrivate/packer_private.h
priv = key.private_bytes(
    encoding=serialization.Encoding.PEM,
    format=serialization.PrivateFormat.TraditionalOpenSSL,
    encryption_algorithm=serialization.NoEncryption()
).decode()
```

Сгенерируйте отдельно для stable (`PrivateKey`), beta (`PrivateBetaKey`) и alpha (`AlphaPrivateKey`).

**⚠️ Приватные ключи нельзя коммитить в публичный репозиторий!**
**⚠️ При смене ключей все установленные клиенты потеряют возможность обновляться — потребуется ручная переустановка.**

### Структура каталогов

```
<your-workspace>/
├── <your-fork>/            # основной репозиторий (форк tdesktop)
├── <your-updates-repo>/    # репозиторий обновлений
├── DesktopPrivate/         # приватные ключи (НЕ публиковать!)
│   ├── packer_private.h    # PrivateKey + PrivateBetaKey
│   └── alpha_private.h     # AlphaPrivateKey
└── Libraries/              # зависимости сборки
```

Путь к `DesktopPrivate` жёстко зашит в `#include` (4 уровня вверх от `_other/packer.cpp`).

---

## Формат current4

```json
{
  "win64": {
    "stable": {
      "released": "6005002",
      "link": "/tx64upd6005002"
    },
    "beta": {
      "released": "6005003",
      "link": "/tx64upd6005003"
    }
  },
  "win": {
    "stable": {
      "released": "6005002",
      "link": "/tupdate6005002"
    }
  },
  "winarm": {
    "stable": {
      "released": "6005002",
      "link": "/tarm64upd6005002"
    }
  },
  "linux": {
    "stable": {
      "released": "6005002",
      "link": "/tlinuxupd6005002"
    }
  },
  "mac": {
    "stable": {
      "released": "6005002",
      "link": "/tmacupd6005002"
    }
  },
  "macarm": {
    "stable": {
      "released": "6005002",
      "link": "/tarmacupd6005002"
    }
  }
}
```

- `released` — целочисленная версия: `major × 1000000 + minor × 1000 + patch`. Пример: `6.5.2` → `6005002`.
- `link` — путь к файлу обновления (начинается с `/`), конкатенируется с `UPDATE_URL`.

Платформенные ключи определяются в `Platform::AutoUpdateKey()`:

| Платформа   | Ключ     | Префикс файла | Пример             |
|-------------|----------|----------------|---------------------|
| Windows x86 | `win`    | `tupdate`      | `tupdate6005002`    |
| Windows x64 | `win64`  | `tx64upd`      | `tx64upd6005002`    |
| Windows ARM | `winarm` | `tarm64upd`    | `tarm64upd6005002`  |
| macOS x64   | `mac`    | `tmacupd`      | `tmacupd6005002`    |
| macOS ARM   | `macarm` | `tarmacupd`    | `tarmacupd6005002`  |
| Linux       | `linux`  | `tlinuxupd`    | `tlinuxupd6005002`  |

---

## Сборка и публикация обновления

### 1. Увеличить версию

Отредактировать `Telegram/build/version`:

```
AppVersion         6005002
AppVersionStrMajor 6.5
AppVersionStrSmall 6.5.2
AppVersionStr      6.5.2
BetaChannel        0
AlphaVersion       0
AppVersionOriginal 6.5.2
```

### 2. Собрать Release

Из соответствующего **Native Tools Command Prompt for VS 2022**:

```bat
cd <your-repo>/Telegram
call configure.bat
cd ..\out
cmake --build . --config Release --target Telegram
```

### 3. Создать пакет обновления

Из `out/Release`:

```bat
:: Windows x64
Packer.exe -version 6005002 -path Telegram.exe -path Updater.exe -path "modules\x64\d3d\d3dcompiler_47.dll" -target win64

:: Windows ARM64 (d3dcompiler не нужен)
Packer.exe -version 6005002 -path Telegram.exe -path Updater.exe -target winarm
```

Packer подписывает пакет приватным ключом из `DesktopPrivate/packer_private.h` и создаёт файл `tx64upd6005002` (или аналог для другой платформы).

### 4. Опубликовать

#### Через publish_update.py

```bash
python Telegram/build/publish_update.py \
  --version 6005002 \
  --platform win64 \
  --file out/Release/tx64upd6005002 \
  --updates-repo path/to/updates-repo

# Для beta-канала добавить --beta
# Для коммита без пуша добавить --no-push
```

Скрипт копирует файл в репо обновлений, обновляет `current4`, коммитит и пушит.

#### Вручную

1. Скопировать файл обновления в репо обновлений.
2. Создать/обновить `current4` (формат см. выше).
3. `git add`, `git commit`, `git push`.

### 5. Проверить

После пуша манифест и файл обновления будут доступны по URL вашего репо.
Клиенты проверяют обновления каждые 8–16 часов. Ручная проверка: **Настройки → Продвинутые → клик по версии**.

---

## Хостинг: GitHub raw vs. Releases

По умолчанию используется `raw.githubusercontent.com` — файлы лежат прямо в репозитории.

**Ограничения:**
- GitHub ограничивает файлы до 100 МБ. Обновления обычно ~40–80 МБ.
- Если файл > 100 МБ, используйте Git LFS: `git lfs track "t*upd*"`.
- Альтернатива — GitHub Releases, но потребуется изменить URL-схему и формат `link` в манифесте на абсолютный URL.

---

## Структура репозитория обновлений

```
<updates-repo>/
├── current4              # JSON-манифест (обязательно)
├── tx64upd6005002        # Windows x64
├── tupdate6005002        # Windows x86
├── tarm64upd6005002      # Windows ARM
├── tlinuxupd6005002      # Linux
├── tmacupd6005002        # macOS x64
├── tarmacupd6005002      # macOS ARM
└── README.md
```

Старые файлы обновлений можно удалять — они нужны только пока есть клиенты на предыдущей версии.
