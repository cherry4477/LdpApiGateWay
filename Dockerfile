FROM gcc:4.8 
RUN mkdir -p /src/myapp 
ENV TIME_ZONE=Asia/Shanghai 
RUN ln -snf /usr/share/zoneinfo/$TIME_ZONE /etc/localtime && echo $TIME_ZONE > /etc/timezone 
COPY . /src/myapp 
WORKDIR /src/myapp 
RUN make clean && make 
EXPOSE 9091 
RUN chmod a+x ./start.sh
CMD ./start.sh 
