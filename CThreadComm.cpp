#include "pch.h"
#include "CThreadComm.h"
#include "ComDlg.h"
#include "Com.h"

CThreadComm::CThreadComm(void)
{
}

CThreadComm::~CThreadComm(void)
{
}

void CThreadComm::runTask(void)
{
	CComDlg *pComDlg = (CComDlg *)theApp.GetMainWnd(); // ��ȡ���Ի���ָ��
	if (pComDlg == NULL)
		return;

	CSerialPort *pSerialPort = (CSerialPort *)GetThreadData(); // ��ȡ������ָ��
	if (pSerialPort == NULL)
		return;

	DWORD dwError = 0;
	COMSTAT comStat;
	memset(&comStat, 0, sizeof(COMSTAT));
	pSerialPort->ClearCommError(&dwError, &comStat);
	
	BOOL bRet = TRUE;
	TCHAR recvBuf[4096] = { 0 }; // ���ջ�����
	TCHAR recvTemp[513] = { 0 }; // ��ʱ������
	DWORD dwLen = 0;
	DWORD dwRead = 0;
	if (comStat.cbInQue > 0) {
		OVERLAPPED overlappedRead;
		memset(&overlappedRead, 0, sizeof(OVERLAPPED));
		overlappedRead.hEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);

		if (comStat.cbInQue < 512) {
			bRet = pSerialPort->ReadFile(recvTemp, comStat.cbInQue, &dwRead, &overlappedRead);
		}
		else {
			bRet = pSerialPort->ReadFile(recvTemp, 512, &dwRead, &overlappedRead);
		}
		
		// �ж��Ƿ����һ�ζ�ȡ
		if (comStat.cbInQue >= dwRead) {
			memcpy(recvBuf + dwLen, recvTemp, dwRead);
			dwLen += dwRead;
		}

		if (overlappedRead.hEvent)
			CloseHandle(overlappedRead.hEvent);

		// �ж��Ƿ��Ѿ���ȡ���ȫ������
		if (comStat.cbInQue == dwRead) {
			dwLen = 0;
			CString szText;
			pComDlg->m_editRECV.GetWindowText(szText);
			szText += recvBuf;
			pComDlg->m_editRECV.SetWindowText(szText);
		}

		// �ж��Ƿ񱻹���
		if (!bRet) {
			if (ERROR_IO_PENDING == GetLastError()) {
				while (!bRet) {
					bRet = pSerialPort->GetOverlappedResult(NULL, &dwRead, TRUE);
					if (GetLastError() != ERROR_IO_INCOMPLETE) {
						pSerialPort->ClearCommError(&dwError, &comStat);
						break;
					}
				}
			}
		}
	}
	Sleep(100);
}
