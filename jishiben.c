#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#define MAX_SIZE 4096
#define MAX_FILES 5

// 主题枚举
enum Theme { LIGHT, DARK, SOLARIZED };

// 多文件结构体
typedef struct {
	char* data;
	size_t length;
	size_t size;
	char filename[256];
	int modified;
} TextFile;

// 全局状态
struct {
	TextFile files[MAX_FILES];
	int current_file;
	int file_count;
	enum Theme current_theme;
} AppState;

// 颜色配置
const int THEME_COLORS[3][4] = {
	// 文字色，背景色，菜单色，高亮色
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
	strcpy(file->filename, "未命名");
}

void expand_buffer(TextFile* file) {
	file->size *= 2;
	file->data = realloc(file->data, file->size);
}

// 新增：主题切换
void switch_theme() {
	AppState.current_theme = (AppState.current_theme + 1) % 3;
	int* colors = THEME_COLORS[AppState.current_theme];
	system("cls");
	set_color(colors[0], colors[1]);
}

// 新增：全文替换
void replace_all(TextFile* file) {
	char old[MAX_SIZE], new[MAX_SIZE];
	printf("输入要替换的文本: ");
	fgets(old, MAX_SIZE, stdin);
	old[strcspn(old, "\n")] = '\0';

	printf("输入新文本: ");
	fgets(new, MAX_SIZE, stdin);
	new[strcspn(new, "\n")] = '\0';

	char* pos = file->data;
	int count = 0;
	size_t old_len = strlen(old);
	size_t new_len = strlen(new);

	while ((pos = strstr(pos, old)) != NULL) {
		// 计算需要扩展的空间
		size_t needed = file->length + new_len - old_len;
		while (needed >= file->size) expand_buffer(file);

		// 移动内存
		memmove(pos + new_len, pos + old_len,
			file->length - (pos - file->data) - old_len);

		// 插入新内容
		memcpy(pos, new, new_len);

		pos += new_len;
		file->length += new_len - old_len;
		count++;
	}

	if (count > 0) {
		file->modified = 1;
		printf("替换完成，共修改 %d 处\n", count);
	}
	else {
		printf("未找到匹配内容\n");
	}
}

// 新增：文件标签显示
void show_tabs() {
	int* colors = THEME_COLORS[AppState.current_theme];
	set_color(colors[2], colors[1]);
	for (int i = 0; i < AppState.file_count; i++) {
		if (i == AppState.current_file) {
			printf("[%d] %s ★ ", i + 1, AppState.files[i].filename);
		}
		else {
			printf(" %d  %s   ", i + 1, AppState.files[i].filename);
		}
	}
	printf("\n");
	set_color(colors[0], colors[1]);
}

int main() {
	// 初始化应用状态
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
		printf(" F1:切换主题 | F2:新建文件 | F3:切换文件 | Ctrl+S:保存 | Ctrl+Q:退出 ");
		set_color(colors[0], colors[1]);
		printf("\n");

		show_tabs(); // 显示文件标签

		printf("\n内容预览（前100字符）:\n");
		set_color(colors[3], colors[1]);
		for (int i = 0; i < (current->length > 100 ? 100 : current->length); i++) {
			putchar(current->data[i]);
		}
		set_color(colors[0], colors[1]);

		printf("\n\n");
		printf("[1] 编辑内容    [2] 保存文件    [3] 全文替换\n");
		printf("[4] 打开文件    [5] 切换主题    [6] 退出\n");
		printf("当前主题：%s | 文件大小：%zu字节\n",
			AppState.current_theme == 0 ? "明亮" :
			AppState.current_theme == 1 ? "暗黑" : "Solarized",
			current->length);

		int choice;
		printf("请选择操作: ");
		scanf("%d", &choice);
		getchar();

		switch (choice) {
		case 1: { // 编辑
			system("cls");
			printf("=== 编辑模式（输入:q返回）===\n");
			printf("%s\n", current->data);

			char input[MAX_SIZE];
			printf("输入新内容: ");
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
		case 2: { // 保存
			char filename[256];
			printf("输入保存文件名: ");
			fgets(filename, sizeof(filename), stdin);
			filename[strcspn(filename, "\n")] = '\0';

			FILE* file = fopen(filename, "w");
			if (file) {
				fwrite(current->data, 1, current->length, file);
				fclose(file);
				strcpy(current->filename, filename);
				current->modified = 0;
				printf("保存成功！\n");
			}
			else {
				printf("保存失败！\n");
			}
			getchar();
			break;
		}
		case 3: // 全文替换
			replace_all(current);
			getchar();
			break;
		case 4: { // 新建文件
			if (AppState.file_count < MAX_FILES) {
				AppState.current_file = AppState.file_count;
				init_file(&AppState.files[AppState.file_count]);
				AppState.file_count++;
			}
			else {
				printf("已达到最大文件数！\n");
			}
			break;
		}
		case 5: // 切换主题
			switch_theme();
			break;
		case 6:
			running = 0;
			break;
		default:
			printf("无效选择！\n");
		}
	}

	// 清理内存
	for (int i = 0; i < AppState.file_count; i++) {
		free(AppState.files[i].data);
	}
	return 0;
}