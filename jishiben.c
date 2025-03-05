#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#define MAX_SIZE 4096
#define MAX_FILES 5

// ����ö��
enum Theme { LIGHT, DARK, SOLARIZED };

// ���ļ��ṹ��
typedef struct {
	char* data;
	size_t length;
	size_t size;
	char filename[256];
	int modified;
} TextFile;

// ȫ��״̬
struct {
	TextFile files[MAX_FILES];
	int current_file;
	int file_count;
	enum Theme current_theme;
} AppState;

// ��ɫ����
const int THEME_COLORS[3][4] = {
	// ����ɫ������ɫ���˵�ɫ������ɫ
	{ 15, 0,   11, 12 },  // Light
	{ 15, 0,   8,  4  },  // Dark
	{ 14, 3,  11, 5  }   // Solarized
};

void set_color(int fg, int bg) {
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, (bg << 4) | fg);
}

void init_file(TextFile* file) {
	file->size = 256;
	file->data = malloc(file->size);
	file->length = 0;
	file->data[0] = '\0';
	file->modified = 0;
	strcpy(file->filename, "δ����");
}

void expand_buffer(TextFile* file) {
	file->size *= 2;
	file->data = realloc(file->data, file->size);
}

// �����������л�
void switch_theme() {
	AppState.current_theme = (AppState.current_theme + 1) % 3;
	int* colors = THEME_COLORS[AppState.current_theme];
	system("cls");
	set_color(colors[0], colors[1]);
}

// ������ȫ���滻
void replace_all(TextFile* file) {
	char old[MAX_SIZE], new[MAX_SIZE];
	printf("����Ҫ�滻���ı�: ");
	fgets(old, MAX_SIZE, stdin);
	old[strcspn(old, "\n")] = '\0';

	printf("�������ı�: ");
	fgets(new, MAX_SIZE, stdin);
	new[strcspn(new, "\n")] = '\0';

	char* pos = file->data;
	int count = 0;
	size_t old_len = strlen(old);
	size_t new_len = strlen(new);

	while ((pos = strstr(pos, old)) != NULL) {
		// ������Ҫ��չ�Ŀռ�
		size_t needed = file->length + new_len - old_len;
		while (needed >= file->size) expand_buffer(file);

		// �ƶ��ڴ�
		memmove(pos + new_len, pos + old_len,
			file->length - (pos - file->data) - old_len);

		// ����������
		memcpy(pos, new, new_len);

		pos += new_len;
		file->length += new_len - old_len;
		count++;
	}

	if (count > 0) {
		file->modified = 1;
		printf("�滻��ɣ����޸� %d ��\n", count);
	}
	else {
		printf("δ�ҵ�ƥ������\n");
	}
}

// �������ļ���ǩ��ʾ
void show_tabs() {
	int* colors = THEME_COLORS[AppState.current_theme];
	set_color(colors[2], colors[1]);
	for (int i = 0; i < AppState.file_count; i++) {
		if (i == AppState.current_file) {
			printf("[%d] %s �� ", i + 1, AppState.files[i].filename);
		}
		else {
			printf(" %d  %s   ", i + 1, AppState.files[i].filename);
		}
	}
	printf("\n");
	set_color(colors[0], colors[1]);
}

int main() {
	// ��ʼ��Ӧ��״̬
	AppState.file_count = 1;
	AppState.current_file = 0;
	AppState.current_theme = LIGHT;
	for (int i = 0; i < MAX_FILES; i++) init_file(&AppState.files[i]);

	int running = 1;
	while (running) {
		TextFile* current = &AppState.files[AppState.current_file];
		int* colors = THEME_COLORS[AppState.current_theme];

		system("cls");
		set_color(colors[2], colors[1]);
		printf(" F1:�л����� | F2:�½��ļ� | F3:�л��ļ� | Ctrl+S:���� | Ctrl+Q:�˳� ");
		set_color(colors[0], colors[1]);
		printf("\n");

		show_tabs(); // ��ʾ�ļ���ǩ

		printf("\n����Ԥ����ǰ100�ַ���:\n");
		set_color(colors[3], colors[1]);
		for (int i = 0; i < (current->length > 100 ? 100 : current->length); i++) {
			putchar(current->data[i]);
		}
		set_color(colors[0], colors[1]);

		printf("\n\n");
		printf("[1] �༭����    [2] �����ļ�    [3] ȫ���滻\n");
		printf("[4] ���ļ�    [5] �л�����    [6] �˳�\n");
		printf("��ǰ���⣺%s | �ļ���С��%zu�ֽ�\n",
			AppState.current_theme == 0 ? "����" :
			AppState.current_theme == 1 ? "����" : "Solarized",
			current->length);

		int choice;
		printf("��ѡ�����: ");
		scanf("%d", &choice);
		getchar();

		switch (choice) {
		case 1: { // �༭
			system("cls");
			printf("=== �༭ģʽ������:q���أ�===\n");
			printf("%s\n", current->data);

			char input[MAX_SIZE];
			printf("����������: ");
			fgets(input, MAX_SIZE, stdin);

			if (strcmp(input, ":q\n") == 0) break;

			size_t input_len = strlen(input);
			while (current->length + input_len >= current->size) {
				expand_buffer(current);
			}

			strcat(current->data, input);
			current->length += input_len;
			current->modified = 1;
			break;
		}
		case 2: { // ����
			char filename[256];
			printf("���뱣���ļ���: ");
			fgets(filename, sizeof(filename), stdin);
			filename[strcspn(filename, "\n")] = '\0';

			FILE* file = fopen(filename, "w");
			if (file) {
				fwrite(current->data, 1, current->length, file);
				fclose(file);
				strcpy(current->filename, filename);
				current->modified = 0;
				printf("����ɹ���\n");
			}
			else {
				printf("����ʧ�ܣ�\n");
			}
			getchar();
			break;
		}
		case 3: // ȫ���滻
			replace_all(current);
			getchar();
			break;
		case 4: { // �½��ļ�
			if (AppState.file_count < MAX_FILES) {
				AppState.current_file = AppState.file_count;
				init_file(&AppState.files[AppState.file_count]);
				AppState.file_count++;
			}
			else {
				printf("�Ѵﵽ����ļ�����\n");
			}
			break;
		}
		case 5: // �л�����
			switch_theme();
			break;
		case 6:
			running = 0;
			break;
		default:
			printf("��Чѡ��\n");
		}
	}

	// �����ڴ�
	for (int i = 0; i < AppState.file_count; i++) {
		free(AppState.files[i].data);
	}
	return 0;
}