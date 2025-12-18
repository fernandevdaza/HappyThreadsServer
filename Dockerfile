# Use an official GCC image to build the application
FROM gcc:latest

# Set the working directory in the container
WORKDIR /app

# Copy the specific source and header directories and other necessary files
# leveraging the .dockerignore to exclude unwanted files
COPY . .

# Build the application
# The Makefile handles creation of bin/ and obj/ directories
RUN make

# Expose the port the server listens on
EXPOSE 8000

# Run the server executable
CMD ["./bin/server"]
