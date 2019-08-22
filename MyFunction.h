#ifndef __MY_FUNCTION_H__
#define __MY_FUNCTION_H__

#include "afxsock.h" // Thư viện Socket
#include "Date.h" // Thư viện Date
#include <fstream>
#include <string>

#define PROXY_SERVER_PORT 8888 // Port của Proxy Server
#define WEB_SERVER_PORT 80 // Port của Web Server
#define GET_DATE Date(10, 7, 2019) // Ngày hiện tại
#define CACHE_FOLDER "Cache//" // Đường dẫn đến thư mục chứa file lưu đối tượng
#define TIME_TO_LIVE 7 // Thời gian sống của một đối tượng

// ------------ Nhóm hàm kiểm tra
bool checkBlockFile(const std::string &url)
{
	std::ifstream inFile("blacklist.conf");
	if (!inFile)
	{
		std::cerr << "Error: Can't open file blacklist.conf :(" << std::endl;
		inFile.close();
		return false;
	}
	std::string temp;
	while (!inFile.eof())
	{
		inFile >> temp; // url
		if (temp != "" && url.find(temp) != std::string::npos)
		{
			inFile.close();
			return true;
		}
	}
	inFile.close();
	return false;
}
bool checkBaseFile(const std::string &url, std::string &fileName, std::string &lastModified)
{
	std::ifstream inFile("base.conf");
	if (!inFile)
	{
		std::cerr << "Error: Can't open file base.conf :(" << std::endl;
		inFile.close();
		return false;
	}
	std::string temp;
	while (!inFile.eof())
	{
		inFile >> temp; // url
		if (temp == url)
		{
			inFile >> fileName; // fileName
			inFile >> temp; // Weekday
			if (temp != "NULL")
			{
				lastModified = temp + ' '; 
				inFile >> temp; lastModified += temp + ' '; // Day
				inFile >> temp; lastModified += temp + ' '; // Month
				inFile >> temp; lastModified += temp + ' '; // Year
				inFile >> temp; lastModified += temp + ' '; // Hour
				inFile >> temp; lastModified += temp; // GMT
			}
			inFile.close();
			return true;
		}
		// Đọc bỏ các kí tự cho đến khi gặp dòng mới
		while (inFile.get() != '\n')
		{
			if (inFile.eof())
			{
				inFile.close();
				return false;
			}
		}
	}
	inFile.close();
	return false;
}
// ------------ Nhóm hàm nhận thông điệp
bool receiveQuery(std::string &query, CSocket &receiverSocket, std::string &method)
{
	char* buf = new char[4096]; // Bộ đệm nhận
	int tmpres; // Chỉ số sequence
	query = "";
	memset(buf, 0, 4096);
	// Nhận gói đầu tiên để kiểm tra
	tmpres = receiverSocket.Receive(buf, 10, 0); // Nhận 10 byte đầu để kiểm tra
	query += std::string(buf, tmpres);
	if (query.find("GET ") != std::string::npos)
		method = "GET";
	else if (query.find("POST ") != std::string::npos)
		method = "POST";
	else
	{
		delete[] buf;
		return false;
	}
	memset(buf, 0, tmpres);
	// Nhận gói các gói còn lại
	while ((tmpres = receiverSocket.Receive(buf, 4096, 0)) > 0)
	{
		query += std::string(buf, tmpres);
		if (std::string(buf).find("\r\n\r\n") != std::string::npos)
			break;
		memset(buf, 0, tmpres);
	}
	delete[] buf;
	if (tmpres < 0)
	{
		std::cerr << "Error: Can't receive query :(" << std::endl;
		return false;
	}
	return true;
}
bool receiveFeedback(std::string &feedback, CSocket &receiverSocket)
{
	char* buf = new char[BUFSIZ]; // Bộ đệm nhận
	int tmpres; // Chỉ số sequence
	feedback = "";
	memset(buf, 0, BUFSIZ);

	while ((tmpres = receiverSocket.Receive(buf, BUFSIZ, 0)) > 0)
	{
		feedback += std::string(buf, tmpres);
		memset(buf, 0, tmpres);
	}
	delete[] buf;
	if (tmpres < 0)
	{
		std::cerr << "Error: Can't receive feedback :(" << std::endl;
		return false;
	}
	return true;
}
// ------------ Nhóm hàm chuyển đổi
wchar_t* charArrayToLPCWSTR(const char* charArray)
{
	int numWString = MultiByteToWideChar(CP_ACP, 0, charArray, -1, NULL, 0);
	wchar_t* wString = new wchar_t[numWString + 1];
	MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, numWString + 1);

	return wString;
}
Date stringToDate(const std::string &str)
{
	int i, day, month, year;
	std::string temp, strDay, strMonth, strYear;
	temp = str; // temp = "Wed, 4 Jul 2007 09:23:24 GMT"

	temp = &temp[5]; // temp = "4 Jul 2007 09:23:24 GMT"
	i = temp.find(' ');
	strDay = temp.substr(0, i);
	day = std::stoi(strDay);

	temp = &temp[i + 1]; // temp = "Jul 2007 09:23:24 GMT"
	i = temp.find(' ');
	strMonth = temp.substr(0, i);
	if (strMonth == "Jan")
		month = 1;
	else if (strMonth == "Feb")
		month = 2;
	else if (strMonth == "Mar")
		month = 3;
	else if (strMonth == "Apr")
		month = 4;
	else if (strMonth == "May")
		month = 5;
	else if (strMonth == "Jun")
		month = 6;
	else if (strMonth == "Jul")
		month = 7;
	else if (strMonth == "Aug")
		month = 8;
	else if (strMonth == "Sep")
		month = 9;
	else if (strMonth == "Oct")
		month = 10;
	else if (strMonth == "Nov")
		month = 11;
	else
		month = 12;

	temp = &temp[i + 1]; // temp = "2007 09:23:24 GMT"
	i = temp.find(' ');
	strYear = temp.substr(0, i);
	year = std::stoi(strYear);

	return Date(day, month, year);
}
// ------------ Nhóm hàm xây dựng thông điệp
std::string buildBlockFeedback()
{
	return "HTTP/1.1 403 Forbidden\r\n\r\n";
}
std::string buildCacheFeedback(const std::string &fileName)
{
	std::ifstream inFile(fileName, std::ios::binary);
	if (!inFile)
	{
		std::cerr << "Error: Can't open file " << fileName << " :(" << std::endl;
		inFile.close();
		return "";
	}

	std::string object;
	inFile.seekg(0, inFile.end);
	size_t endPos = inFile.tellg();
	inFile.seekg(0, inFile.beg);
	char* buf = new char[endPos];
	memset(buf, 0, endPos);
	inFile.read(buf, endPos);
	object = std::string(buf, endPos);
	delete[] buf;

	return object;
}
std::string buildModifiedQuery(const std::string &page, const std::string &host, const std::string &lastModified)
{
	return "GET " + page + " HTTP/1.1\r\nHost: " + host + "\r\nIf-modified-since: " + lastModified + "\r\n\r\n";
}
// ------------ Nhóm hàm lấy giá trị
char* getIP(const char* host)
{
	struct hostent* hent;
	int numIP = 15; // "XXX.XXX.XXX.XXX"
	char* ip = new char[numIP + 1];
	memset(ip, 0, numIP + 1);

	if ((hent = gethostbyname(host)) == NULL)
	{
		std::cerr << "Error: Can't convert host :(" << std::endl;
		return "";
	}
	if (inet_ntop(AF_INET, (void*)hent->h_addr_list[0], ip, numIP) == NULL)
	{
		std::cerr << "Error: Can't resolve host :(" << std::endl;
		return "";
	}
	return ip;
}
std::string getHost(const std::string &query)
{
	int iFirst = query.find("Host: ") + 6; // vị trí "bắt đầu" của trường host trong chuỗi query
	int iLast = iFirst + std::string(&query[iFirst]).find("\r\n") - 1; // vị trí "kết thúc" của trường host trong chuỗi query
	return query.substr(iFirst, iLast - iFirst + 1);
}
std::string getPage(const std::string &query)
{
	int iFirst = query.find(' ') + 1; // vị trí "bắt đầu" của trường page trong chuỗi query
	int iLast = query.find(" HTTP/") - 1; // vị trí "kết thúc" của trường page trong chuỗi query
	return query.substr(iFirst, iLast - iFirst + 1);
}
std::string getLastModified(const std::string &feedback)
{
	int iFirst = feedback.find("Last-Modified: ") + 15;
	if (iFirst != std::string::npos + 15)
	{
		int iLast = iFirst + std::string(&feedback[iFirst]).find("\r\n") - 1;
		return feedback.substr(iFirst, iLast - iFirst + 1);
	}
	return "NULL";
}
std::string getObjectFileName()
{
	std::ifstream inFile("base.conf");
	if (!inFile)
	{
		std::cerr << "Error: can't open file base.conf :(" << std::endl;
		inFile.close();
		return "";
	}
	char temp[2048];
	int countLine = 0;
	// Đếm số dòng của file
	while (!inFile.eof())
	{
		inFile.getline(temp, 2048);
		if (strcmp(temp, "") == 0)
			break;
		countLine++;
	}
	if (countLine == 0)
	{
		inFile.close();
		return "0.conf";
	}
	inFile.close();
	inFile.open("base.conf");
	while (countLine != 1)
	{
		inFile.getline(temp, 2048);
		countLine--;
	}
	inFile >> temp;
	inFile >> temp;
	// Bỏ đuôi .conf
	int iLastTemp = std::string(temp).find('.');
	int num = std::stoi(std::string(temp).substr(0, iLastTemp));
	num++;
	itoa(num, temp, 10);
	inFile.close();
	return std::string(temp) + ".conf";
}
std::string getDateSaveFile(const std::string &fileName)
{
	std::string object = buildCacheFeedback(fileName);
	int iFirst = object.find("Date: ") + 6; // vị trí "bắt đầu" của trường date trong chuỗi object
	if (iFirst == std::string::npos + 6)
		return "";
	int iLast = iFirst + std::string(&object[iFirst]).find("\r\n") - 1; // vị trí "kết thúc" của trường date trong chuỗi object
	return object.substr(iFirst, iLast - iFirst + 1);
}
// ------------ Nhóm hàm thêm/lưu/xóa/cập nhật đối tượng xuống file
bool writeBaseFile(const std::string &url, const std::string &fileName, const std::string &lastModified)
{
	std::ofstream outFile("base.conf", std::ios::app);
	if (!outFile)
	{
		std::cerr << "Error: Can't open file base.conf :(" << std::endl;
		return false;
	}
	outFile << url << "\t" << fileName << "\t" << lastModified << "\n";
	outFile.close();
	return true;
}
bool saveObject(const std::string &object, const std::string &fileName)
{
	std::ofstream outFile(fileName, std::ios::binary);
	if (!outFile)
	{
		std::cerr << "Error: Can't open file " << fileName << " :(" << std::endl;
		return false;
	}
	outFile.write(object.c_str(), object.size());
	outFile.close();
	return true;
}
void deleteFileOverTimeToLive()
{
	int index = 0;
	std::string fileName;
	char* temp = new char[2048];
	std::ifstream oldFile;
	std::ofstream newFile;

	while (true)
	{
		oldFile.open("base.conf");
		memset(temp, 0, 2048);
		for (int j = 0; j <= index; j++)
			oldFile.getline(temp, 2048);
		if (std::string(temp) == "")
			break;
		oldFile.close();

		int iFirst = std::string(temp).find('\t') + 1;
		int iLast = iFirst + std::string(&temp[iFirst]).find('\t') - 1;
		fileName = std::string(temp).substr(iFirst, iLast - iFirst + 1);

		if ((getDateSaveFile(CACHE_FOLDER + fileName) != "" && GET_DATE - stringToDate(getDateSaveFile(CACHE_FOLDER + fileName)) > 7) ||
			getDateSaveFile(CACHE_FOLDER + fileName) == "")
		{
			oldFile.open("base.conf");
			newFile.open("replica.conf", std::ios::app);
			int count = 0;

			while (!oldFile.eof())
			{
				memset(temp, 0, 2048);
				oldFile.getline(temp, 2048);
				if (std::string(temp) == "")
					break;
				if (count != index)
					newFile << temp << '\n';
				count++;
			}
			oldFile.close();
			newFile.close();
			system("del base.conf");
			rename("replica.conf", "base.conf");
			std::cout << "Deleted file " << fileName << " successfully :)" << std::endl;
			fileName = "del Cache\\" + fileName;
			system(fileName.c_str());
		}
		else
			index++;
	}
	oldFile.close();
	delete[] temp;
}
bool updateLastModified(const std::string &url, const std::string &lastModified)
{
	std::fstream file("base.conf");
	if (!file)
	{
		std::cerr << "Error: Can't open file base.conf :(" << std::endl;
		return false;
	}
	std::string temp;
	file >> temp; // url
	while (temp != url)
	{
		file >> temp; // fileName
		file >> temp; // Weekday
		if (temp != "NULL")
		{
			file >> temp; // Day
			file >> temp; // Month
			file >> temp; // Year
			file >> temp; // Hour
			file >> temp; // GTM
		}
		file >> temp; // url
	}
	file >> temp; // fileName
	size_t pos = (size_t)file.tellg() + 1;
	file.seekp(pos, file.beg);
	file << lastModified;
	return true;
}
// ------------ Hàm tiểu trình
DWORD WINAPI Thread(LPVOID lp)
{
	// 1. Ép kiểu tham số truyền vào
	SOCKET* param = (SOCKET*)lp;
	// 2. Khai báo các biến 
	CSocket connectorSocket; // Socket giữ kết nối với Browser
	CSocket clientSocket; // Socket kết nối với Web Server
	CSocket modifiedSocket; // Socket kết nối với Web Server (để cập nhật trang Web)
	std::string host = ""; // Tên miền của Web Server
	std::string page = ""; // Tên trang web truy xuất từ Web Server
	std::string url = ""; // url = host + page
	std::string ip = ""; // Địa chỉ IP được phân giải từ tên miền Web Server
	std::string query = ""; // Thông điệp yêu cầu
	std::string feedback = ""; // Thông điệp phản hồi
	std::string fileName = ""; // Tên của file lưu đối tượng trang web vào cache
	std::string lastModified = "NULL"; // Chứa chuỗi thời điểm trang Web cập nhật lần cuối -> Dùng trong phần cập nhật trang Web có điều kiện
	std::string method = ""; // Phương thức của thông điệp yêu cầu
	// 3. Chuyển SOCKET -> CSOCKET
	connectorSocket.Attach(*param);
	// 4. Nhận thông điệp yêu cầu từ Browser 
	if (receiveQuery(query, connectorSocket, method) == false)
		goto endLabel;
	std::cout << "------------------------------------- QUERY -------------------------------------" << std::endl;
	std::cout << "<START>" << std::endl << std::endl;
	std::cout << query << std::endl << std::endl;
	std::cout << "<END>" << std::endl << std::endl;
	// 5. Tách 2 trường host, page từ query và ghép lại thành url
	if (query == "")
		goto endLabel;
	host = getHost(query);
	page = getPage(query);
	url = page;
	// 6. Kiểm tra url có nằm trong danh sách cấm của file blacklist.conf hay không?
	if (checkBlockFile(url))
	{
		// 6.1 Xây dựng thông điệp phản hồi cấm
		feedback = buildBlockFeedback();
		std::cout << "------------------------------------- BLOCK FEEDBACK -------------------------------------" << std::endl;
		std::cout << "<START>" << std::endl << std::endl;
		std::cout << feedback << std::endl << std::endl;
		std::cout << "<END>" << std::endl << std::endl;
		// 6.2 Gửi thông điệp phản hồi cấm cho Browser
		connectorSocket.Send(feedback.c_str(), feedback.size(), 0);
		goto endLabel;
	}
	// 7. Nếu là phương thức POST thì gửi luôn thông điệp yêu cầu lên Web Server và không Caching
	if (method == "POST")
		goto firstLabel;
	// 8. Kiểm tra url có nằm trong danh sách url đã truy cập được lưu trong file base.conf hay không?
	if (checkBaseFile(url, fileName, lastModified))
	{
		// 8.1 Kiểm tra việc cập nhật trang Web
		if (lastModified != "NULL")
		{
			if (GET_DATE - stringToDate(lastModified) > 7)
			{
				// 8.1.1 Xây dựng thông điệp yêu cầu cập nhật trang Web có điều kiệu
				std::string modified = buildModifiedQuery(page, host, lastModified);
				// 8.1.2 Tạo Socket Modified để gửi thông điệp yêu cầu cập nhật cho Web Server
				modifiedSocket.Create();
				ip = getIP(host.c_str());
				if (ip == "")
					goto endLabel;
				std::cout << "Web Server's IP: " << ip << std::endl;
				if (modifiedSocket.Connect(charArrayToLPCWSTR(ip.c_str()), WEB_SERVER_PORT) < 0)
				{
					std::cerr << "Error: Can't connect to the Web Server :(" << std::endl;
					goto endLabel;
				}
				std::cout << "Successfully connected to the Web Server :)" << std::endl << std::endl;
				// 8.1.3 Gửi thông điệp yêu cầu cập nhật lên Web Server
				modifiedSocket.Send(modified.c_str(), modified.size() + 1, 0);
				std::cout << "------------------------------------- MODIFIED QUERY -------------------------------------" << std::endl;
				std::cout << "<START>" << std::endl << std::endl;
				std::cout << modified << std::endl << std::endl;
				std::cout << "<END>" << std::endl << std::endl;
				// 8.1.4 Nhận thông điệp phản hồi cập nhật từ Web Server
				if (receiveFeedback(feedback, modifiedSocket) == false)
					goto endLabel;
				std::cout << "------------------------------------- MODIFIED FEEDBACK -------------------------------------" << std::endl;
				std::cout << "<START>" << std::endl << std::endl;
				std::cout << feedback << std::endl << std::endl;
				std::cout << "<END>" << std::endl << std::endl;
				// 8.1.5 Kiểm tra thông điệp phản hồi có yêu cầu cập nhật không?
				if (feedback.find("304 Not Modified") == std::string::npos)
				{
					// 8.1.5.1 Gửi thông điệp yêu cầu của Browser lên Web Server
					modifiedSocket.Send(query.c_str(), query.size() + 1, 0);
					// 8.1.5.2 Nhận thông điệp phản hồi từ Web Server
					if (receiveFeedback(feedback, modifiedSocket) == false)
						goto endLabel;
					std::cout << "------------------------------------- WEB SERVER FEEDBACK -------------------------------------" << std::endl;
					std::cout << "<START>" << std::endl << std::endl;
					std::cout << "..." << std::endl << std::endl;
					std::cout << "<END>" << std::endl << std::endl;
					// 8.1.5.3 Gửi thông điệp phản hồi cho Browser
					if (feedback == "")
						goto endLabel;
					connectorSocket.Send(feedback.c_str(), feedback.size(), 0);
					// 8.1.5.4 Tách phần lastModified từ feedback và update
					lastModified = getLastModified(feedback);
					if (updateLastModified(url, lastModified) == false)
						goto endLabel;
					// 8.1.5.5 Lưu object mới cập nhật xuống file chứa object cũ
					saveObject(feedback, CACHE_FOLDER + fileName);
					// 8.1.5.6 Đóng kết nối với Web Server
					modifiedSocket.Close();
					goto endLabel;
				}
			}
		}
		// 8.2 Truy xuất file "fileName.conf" để lấy đối tượng
		feedback = buildCacheFeedback(CACHE_FOLDER + fileName);
		std::cout << "------------------------------------- CACHE FEEDBACK -------------------------------------" << std::endl;
		std::cout << "<START>" << std::endl << std::endl;
		std::cout << "..." << std::endl << std::endl;
		std::cout << "<END>" << std::endl << std::endl;
		// 8.3 Gửi lại Browser
		connectorSocket.Send(feedback.c_str(), feedback.size(), 0);
		goto endLabel;
	}

firstLabel:

	// 9. Tạo Socket Client để forward thông điệp yêu cầu của Browser cho Web Server
	clientSocket.Create();
	ip = getIP(host.c_str());
	if (ip == "")
		goto endLabel;
	std::cout << "Web Server's IP: " << ip << std::endl;
	if (clientSocket.Connect(charArrayToLPCWSTR(ip.c_str()), WEB_SERVER_PORT) < 0)
	{
		std::cerr << "Error: Can't connect to the Web Server :(" << std::endl;
		goto endLabel;
	}
	std::cout << "Successfully connected to the Web Server :)" << std::endl << std::endl;
	// 10. Gửi thông điệp yêu cầu của Browser lên Web Server
	clientSocket.Send(query.c_str(), query.size() + 1, 0);
	// 11. Nhận thông điệp phản hồi từ Web Server
	if (receiveFeedback(feedback, clientSocket) == false)
		goto endLabel;
	std::cout << "------------------------------------- WEB SERVER FEEDBACK -------------------------------------" << std::endl;
	std::cout << "<START>" << std::endl << std::endl;
	std::cout << "..." << std::endl << std::endl;
	std::cout << "<END>" << std::endl << std::endl;
	// 12. Gửi thông điệp phản hồi cho Browser
	if (feedback == "")
		goto endLabel;
	connectorSocket.Send(feedback.c_str(), feedback.size(), 0);
	if (method == "GET")
	{
		// 13. Tách phần lastModified từ feedback
		lastModified = getLastModified(feedback);
		// 14. Phát sinh tên file lưu object
		fileName = getObjectFileName();
		// 15. Thêm 1 bộ mới vào file base.conf
		writeBaseFile(url, fileName, lastModified);
		// 16. Lưu object xuống một file mới
		saveObject(feedback, CACHE_FOLDER + fileName);
	}
	// 17. Đóng kết nối với Web Server
	clientSocket.Close();

endLabel:

	// 18. Đóng kết nối với Browser
	connectorSocket.Close();
	// 19. Huỷ tài nguyên
	delete param;
	return 0;
}

#endif