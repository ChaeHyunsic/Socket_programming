#include <windows.h>
#include <stdio.h>

#define MAXCNT 100000000
int g_count = 0;

HANDLE addHandle;
HANDLE subHandle;

DWORD WINAPI MyThread1(LPVOID arg)
{
	DWORD retval;

	for (int i = 0; i < MAXCNT; i++) {
		// ���� �Ϸ� ���
		retval = WaitForSingleObject(subHandle, INFINITE);
		if (retval != WAIT_OBJECT_0) break;

		// ���� ������ ���� ����
		g_count += 2;

		// ���� �Ϸ� �˸�
		SetEvent(addHandle);
	}
	return 0;
}

DWORD WINAPI MyThread2(LPVOID arg)
{
	DWORD retval;

	for (int i = 0; i < MAXCNT; i++) {
		// ���� �Ϸ� ���
		retval = WaitForSingleObject(addHandle, INFINITE);
		if (retval != WAIT_OBJECT_0) break;

		// ���� ������ ���� ����
		g_count -= 2;

		// ���� �Ϸ� �˸�
		SetEvent(subHandle);
	}
	return 0;
}

int main(int argc, char* argv[])
{
	// �� ���� �ڵ� ���� �̺�Ʈ ����(���� ���ȣ, ��ȣ ����)
	addHandle = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (addHandle == NULL) return 1;

	subHandle = CreateEvent(NULL, FALSE, TRUE, NULL);
	if (addHandle == NULL) return 1;

	// �� ���� ������ ����
	HANDLE hThread[2];
	hThread[0] = CreateThread(NULL, 0, MyThread1, NULL, 0, NULL);
	hThread[1] = CreateThread(NULL, 0, MyThread2, NULL, 0, NULL);

	// �� ���� ������ ���� ���
	WaitForMultipleObjects(2, hThread, TRUE, INFINITE);

	// ��� ���
	printf("g_count = %d\n", g_count);

	// �̺�Ʈ ����
	CloseHandle(addHandle);
	CloseHandle(subHandle);

	return 0;
}