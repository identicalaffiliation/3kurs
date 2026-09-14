import subprocess


def apply_filter(file, filtergraph, mode="preview"):
    if mode == "preview":
        subprocess.run([
            "ffplay",
            "-i", file,
            "-vf", filtergraph,
            "-autoexit"
        ])

    elif mode == "save":
        print("Начинаем сохранение файла.")

        subprocess.run([
            "ffmpeg",
            "-y",
            "-i", file,
            "-filter_complex", filtergraph,
            "-c:a", "copy",
            "result.mp4"
        ])

filters = {
    "1": {
        "name": "Поворот цвета",
        "filtergraph": "hue=h=90"
    },
    "2": {
        "name": "Негатив",
        "filtergraph": "negate"
    },
    "3": {
        "name": "Чёрно-белое",
        "filtergraph": "format=gray"
    },
    "4": {
        "name": "Негатив + чёрно-белое",
        "filtergraph": "negate,format=gray"
    }
}

filename = "funny_dog.MP4"
selected_filter = "null"

while True:
    print("\nДоступные фильтры:")
    for key, value in filters.items():
        print(f"{key}. {value['name']}")
    choice = input(
        "\nВыберите фильтр для предпросмотра "
        "(save — сохранить, quit — выйти): "
    )
    if choice in filters:
        selected_filter = filters[choice]["filtergraph"]
        print(
            f"Запускаем предпросмотр: "
            f"{filters[choice]['name']}"
        )
        print(f"Filtergraph: {selected_filter}")
        apply_filter(
            filename,
            selected_filter,
            mode="preview"
        )

    elif choice == "save":
        if selected_filter == "null":
            print("Сначала выберите фильтр.")
            continue
        apply_filter(
            filename,
            selected_filter,
            mode="save"
        )
        print("Видео сохранено в result.mp4")
        break

    elif choice == "quit":
        break

    else:
        print("Неизвестная команда.")